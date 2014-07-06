
#include "db.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void filenames_from_dbname(char* name,
													 char** idx_n,
													 char** data_n) {
	*idx_n = malloc(sizeof(char) * strlen(name) + 4),
	*data_n = malloc(sizeof(char) * strlen(name) + 4);
	
	sprintf(*idx_n, "%s.idx", name);
	sprintf(*data_n, "%s.dat", name);	
}

int create_db(char* name) {
	char *idx_n, *data_n;

	filenames_from_dbname(name, &idx_n, &data_n);
		
	FILE 
		*idx  = fopen(idx_n, "w"), 
		*data = fopen(data_n, "w");
	
	if(NULL == idx) {
		fprintf(stderr, "Error opening idx, %s\n",
						strerror(errno));
		exit(1);
	}

	if(NULL == data) {
		fprintf(stderr, "Error opening dat, %s\n",
						strerror(errno));
		exit(1);
	}
	
	// init idx
	struct idx_header hdr;
	hdr.current = 0;
	hdr.idx_ptr = sizeof(hdr);
	hdr.db_ptr = 0;
	
	clearerr(idx);
	if(0 != fseek(idx, 0, 0)) {
		fprintf(stderr, "Error moving to beginning of idx, %s\n",
						strerror(errno));
		exit(2);
	}
	
	clearerr(idx);	
	size_t sz = fwrite(&hdr, sizeof(hdr), 1, idx);
	if(sz != 1) {
		fprintf(stderr, "Not everything was written, got %d "
						"ferror = %d\n",
						sz, ferror(idx));
		exit(1);
	}
	
	fclose(idx);
	fclose(data);
	
	return 0;
}

int load_db(char* name, struct db* loaded) {
	// open index and data file
	char *idx_n, *data_n;

	filenames_from_dbname(name, &idx_n, &data_n);
		
	FILE 
		*idx  = fopen(idx_n, "r+"), 
		*data = fopen(data_n, "r+");
	
	if(NULL == idx) {
		fprintf(stderr, "Error on fopen");
		exit(1);
	}

	if(NULL == data) {
		fprintf(stderr, "Error on fopen");
		exit(1);
	}
	
	loaded->idx = idx;
	loaded->data = data;
	
	size_t r = fread(&(loaded->hdr), sizeof(loaded->hdr), 1, idx);
	if(r != 1) {
		fprintf(stderr, "Error reading idx header, got %d expected %d\n",
						r, sizeof(loaded->hdr));
		exit(2);
	}
	
	return 0;
}

int get_from_db(struct db* db, long long id, 
								 void** data, size_t* len) {
	*len = 0;
	*data = NULL;
	
	// find it in idx
	fseek(db->idx, sizeof(struct idx_header), 0);
	struct idx_entry entry;
	size_t sz; 

 _read:
	
	sz = fread(&entry, sizeof(entry), 1, db->idx);
	if(sz != 1) {
		if(feof(db->idx)) {
			// not found
			return 1;
		}
		if(ferror(db->idx)) {
			fprintf(stderr, "Error reading idx\n");
			exit(3);
		}
	}
	
	// if it's found, read it from .dat!
	if(entry.id == id) {
		fseek(db->data, entry.offset, 0);
		char* buffer = (char*) malloc(entry.len);
		sz = fread(buffer, entry.len, 1, db->data);
		if(sz != 1) {
			fprintf(stderr, "Error reading dat, got %d\n", sz);
			exit(4);
		}
		
		*len = entry.len;
		*data = buffer;
		
		return 0;
	}
	
	goto _read;
	
	return 1;
}

int put_in_db(struct db* db, long long *id,
							 void* data, size_t len) {
	// update current index in .idx
	++(db->hdr.current);
	
	// add to .dat
	long data_ptr = db->hdr.db_ptr;
	fseek(db->data, data_ptr, 0);
	fwrite(data, len, 1, db->data);
		
	// update header
	db->hdr.db_ptr = ftell(db->data);
	
	// add to .idx
	fseek(db->idx, db->hdr.idx_ptr, 0);
	struct idx_entry entry;
	entry.id = db->hdr.current;
	entry.len = len;
	entry.offset = data_ptr;
	fwrite(&entry, sizeof(entry), 1, db->idx);
	
	// update header
	db->hdr.idx_ptr = ftell(db->idx);

	// write header
	fseek(db->idx, 0, 0);
	fwrite(&(db->hdr), sizeof(struct idx_header), 1, db->idx);
	
	*id = db->hdr.current;

	return 0;
}

int close_db(struct db* db) {
	fclose(db->idx);
	fclose(db->data);
	return 0;
}
