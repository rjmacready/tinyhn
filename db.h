#ifndef _DB_H_
#define _DB_H_

#include <stdio.h>

struct idx_entry {
	long long id;
	size_t len;
	long offset;	
};

struct idx_header {
	long long current;
	long idx_ptr;
	long db_ptr;
};

struct db {
	FILE* idx;
	FILE* data;
	struct idx_header hdr;
};

int create_db(char* name);
int load_db(char* name, struct db* loaded);
int put_in_db(struct db* db, long long *id, 
							 void* data, size_t len);

int get_from_db(struct db* db, long long id, 
								void** data, size_t* len);

int close_db(struct db* db);

#endif
