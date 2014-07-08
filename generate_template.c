#include <stdlib.h>
#include <stdio.h>

#include "header_utils.h"

int main(int argc, char** argv) {
	FILE* f = fopen(argv[1], "r");
	if(NULL == f) {
		fprintf(stderr, "Error on fopen\n");
		exit(1);
	}
	
	char* outfilename = argv[2];
	
	char* h_file_n = malloc(sizeof(char) * strlen(outfilename) + 3);
	sprintf(h_file_n, "%s.h", outfilename);
	FILE* h_file = fopen(h_file_n, "w");
	if(NULL == h_file) {
		fprintf(stderr, "Error on fopen .h\n");
		exit(1);
	}
	
	char* c_file_n = malloc(sizeof(char) * strlen(outfilename) + 3);
	sprintf(c_file_n, "%s.c", outfilename);
	FILE* c_file = fopen(c_file_n, "w");
	if(NULL == c_file) {
		fprintf(stderr, "Error on fopen .c\n");
		exit(1);
	}
	
	char* h_const = filename_to_header(outfilename);
		
	
	//	char nfst = 1; // just to control commas in html mode
	char mode = 0; // html / C
	char c; // char from file

	// mode sequence
	int mode_i = 0; 
	int modes[100] = {-1};
	int h_i = 0, h_len[50] = {0};

	// functions
	char fns[50][50] = {{0}};
	int fn_i = 0, fn_j;
	
	fprintf(c_file, "#include <stdlib.h>\n");
	fprintf(c_file, "#include <stdio.h>\n");
	fprintf(c_file, "#include <string.h>\n\n");
	fprintf(c_file, "#include \"response.h\"\n\n");

	fprintf(c_file, "char RSC_%s_0[] = {\n\t", h_const);	
	modes[mode_i++] = mode;
	
	while(fread(&c, sizeof(char), 1, f)) {
		
		if(c == '$') {
			if(mode == 0) {
				// Enter C mode
				fn_j = 0;
				++h_i;
				fprintf(c_file, "0x0};\n");
				mode = 1;
			} else if(mode == 1) {
				// Enter html mode
				++fn_i;
				fprintf(c_file, "char RSC_%s_%d[] = {\n\t", h_const, fn_i);	
				mode = 0;
			}
			modes[mode_i++] = mode;
			//nfst = 1;
			continue;
		}/* else if(mode == 0) {
			if(nfst == 0) {
				fprintf(fo, ", ");
			} else {
				nfst = 0;
			}
			}*/
		
		if(mode == 0) {
			// HTML mode
			fprintf(c_file, "0x%x, ", c & 0xff);
			++h_len[h_i];
		} else if(mode == 1) {
			// C mode		
			fns[fn_i][fn_j++] = c; 
		}
	}
	
	fprintf(c_file, "0x0};\n\n");

	fprintf(h_file, "#ifndef _RSC_%s_H_\n", h_const);
	fprintf(h_file, "#define _RSC_%s_H_\n\n", h_const);
	fprintf(h_file, "#include \"response.h\"\n\n");
	fprintf(h_file, "int resource_%s(struct resp_string** r, int* resp_no);\n", h_const);
	
	int i, j, i_f, i_h, x;
	for(i = 0; i < fn_i; ++i) {
		x = 0;
		for(j = 0; j < i; ++j) {
			if(0 == strcmp(fns[i], fns[j])) {
				x = 1;
			}
		}
		
		if(1 == x) {
			continue;
		}
		
 		fprintf(c_file, "extern char* %s_%s();\n\n", fns[i], h_const);		
	}
	
	fprintf(c_file, "int resource_%s(struct resp_string** resp, int* resp_no) {\n", h_const);
	fprintf(c_file, 
					"\tstruct resp_string* o = "
					"(struct resp_string*) malloc(sizeof(struct resp_string) * %d);\n",
					mode_i + fn_i);
	
	fprintf(c_file, "\tint count = 0;\n");
	for(i_f = 0, i_h = 0, i = 0; i < mode_i; ++i) {
		fprintf(c_file, "\n");
		if(modes[i] == 0) {
			// html
			fprintf(c_file, "\to[%d].buffer = RSC_%s_%d;\n", i, h_const, i_h);
			fprintf(c_file, "\to[%d].len = %d;\n", i, h_len[i_h]);
			fprintf(c_file, "\tcount += %d;\n", h_len[i_h]);
			++i_h;
		} else if(modes[i] == 1) {
			// C
			fprintf(c_file, "\to[%d].buffer = %s_%s();\n", i, fns[i_f], h_const);
			fprintf(c_file, "\to[%d].len = strlen(o[%d].buffer);\n", i, i);
			fprintf(c_file, "\tcount += o[%d].len;\n", i);
			++i_f;
		}
	}
	fprintf(c_file, "\t*resp = o;\n");
	fprintf(c_file, "\t*resp_no = %d;\n", i);
	fprintf(c_file, "\treturn count;\n");
	fprintf(c_file, "}\n");
	
	fprintf(h_file, "#endif\n");
	fclose(h_file);
	fclose(c_file);
	fclose(f);
	return 0;
}
