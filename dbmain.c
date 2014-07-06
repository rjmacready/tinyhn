#include <string.h>
#include <stdlib.h>

#include "db.h"

int main(int argc, char** argv) {
	char action = argv[1][0];
	char* name = argv[2];
	struct db db;
	long long id;

	if(action == 'c') {
		if(0 == create_db(name)) {
			fprintf(stderr, "Created db\n");
		} else {
			fprintf(stderr, "Some problem\n");
			exit(2);
		}
	}
	
	if(action == 'o' || action == 'p' || action == 'g') {		
		if(0 == load_db(name, &db)) {
			fprintf(stderr, "Loaded db\n");
		} else {
			fprintf(stderr, "Some problem\n");
			exit(2);
		}
	}
	
	if(action == 'p') {
		char *data = argv[3];
		size_t len = strlen(data);
		
		if(0 == put_in_db(&db, &id, data, len)) {
			fprintf(stderr, "Saved with id %lld\n", id);
		} else {
			fprintf(stderr, "Some problem\n");
			exit(2);
		}
	}
	
	if(action == 'g') {
		char *data;
		size_t len;
		id = strtoll(argv[3], NULL, 10);
		
		if(0 == get_from_db(&db, id,(void**) &data, &len)) {
			fprintf(stderr, "Loaded from db:[%lld] '%s' (len: %d)\n", id, data, len);
		} else {
			fprintf(stderr, "Not found\n");
		}
	}

	if(action == 'o' || action == 'p' || action == 'g') {
		if(0 == close_db(&db)) {
			fprintf(stderr, "Closed\n");
		} else {
			fprintf(stderr, "Some problem\n");
			exit(2);
		}
	}
	
	return 0;
}
