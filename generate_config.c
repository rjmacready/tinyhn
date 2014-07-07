#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	FILE* fc = fopen(argv[1], "r");
	if(NULL == fc) {
		fprintf(stderr, "Error on fopen\n");
		exit(1);
	}

	char config[500][3][500];
	int cnf = 0;
	int c_i[3], mode = 0;
	char c;
	
	memset(c_i, 0, sizeof(int) * 3);
	memset(config, 0, sizeof(char) * 500 * 3 * 500);
	
	while(fread(&c, sizeof(char), 1, fc)) {
		if(c == ',') {
			++mode;
			if(mode > 2) {
				fprintf(stderr, "Too many columns\n");
				exit(3);
			}
			
			continue;
		} else if(c == '\n') {
			++cnf;
			mode = 0;
			
			continue;
		}
		
		// some validations
		if(mode == 0 && c == ' ') {
			fprintf(stderr, "Column %d can't have '%c'\n", mode, c);
			exit(3);
		}
		
		config[cnf][mode][c_i[mode]++] = c;
		if(c_i[mode] == 500) {
			fprintf(stderr, "value too big\n");
			exit(3);
		}
	}
	fclose(fc);

	FILE* h_out = fopen("configs.h", "w");
	if(NULL == h_out) {
		fprintf(stderr, "Error opening configs.h for writing\n");
		exit(4);
	}
	fprintf(h_out, "#ifndef _CONFIG_H_\n");
	fprintf(h_out, "#define _CONFIG_H_\n\n");

	int i, j;
	for(i=0; i < cnf; ++i) {
		fprintf(h_out, "const static ");

		if(0 == strcmp(config[i][2], "char[]")) {
			fprintf(h_out, "char");
		} else {
			fprintf(stderr, "Unexpected type %s", config[i][2]);
			exit(5);
		}

		fprintf(h_out, " CONFIG_%s", 
						config[i][0]);
		
		if(0 == strcmp(config[i][2], "char[]")) {
			fprintf(h_out, "[]");
		} else {
			fprintf(stderr, "Unexpected type %s", config[i][2]);
			exit(5);
		}

		fprintf(h_out, " = ");
		
		if(0 == strcmp(config[i][2], "char[]")) {
			fprintf(h_out, "{");
			for(j=0; config[i][1][j] != '\0'; ++j) {
				fprintf(h_out, "0x%x, ", config[i][1][j] & 0xFF);
			}
			fprintf(h_out, "0x0};\n");
		} else {
			fprintf(stderr, "Unexpected type %s\n", config[i][2]);
			exit(5);
		}
	}
	fprintf(h_out, "\n#endif\n");
	fclose(h_out);
	
	return 0;
}
