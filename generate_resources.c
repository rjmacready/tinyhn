#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include "header_utils.h"

int main(int argc, char** argv) {
	FILE* f = fopen(argv[1], "r");
	if(NULL == f) {
		fprintf(stderr, "Error on fopen\n");
		exit(1);
	}
	
	char* outfilename = argv[2];
	FILE* fo = fopen(outfilename, "w");
	if(NULL == fo) {
		fprintf(stderr, "Error on fopen 2\n");
		exit(2);
	}
	
	char* headerconst = filename_to_header(outfilename);
	
	fprintf(fo, "#ifndef _DEF_RSRC_%s_H_\n", headerconst);
	fprintf(fo, "#define _DEF_RSRC_%s_H_\n", headerconst);
	fprintf(fo, "\n");
	
	fprintf(fo, "static char RSRC_%s[] = {", headerconst);
	char c; int i = 0;

	while(fread(&c, sizeof(c), 1, f)) {
		if((i++ % 10) == 0) {
			fprintf(fo, "\n\t");
		}
		
		fprintf(fo, "0x%x, ", c & 0xff);
	}
	fprintf(fo, "0x0\n};\n");
	
	fprintf(fo, "\n");
	fprintf(fo, "#endif\n");
	
	fclose(fo);
	fclose(f);
	
	/* DIR* dir = opendir("resources");
	struct dirent *fs;
	
	while(fs = readdir(dir)) {
		if(fs->d_type != DT_REG) {
			continue; 
		}
		
		fprintf(stderr, "a entry (%d) (%s)\n", fs->d_ino, fs->d_name);
		
	}
	
	closedir(dir);
	*/
	return 0;
}
