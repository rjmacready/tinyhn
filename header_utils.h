#ifndef _HEADER_UTILS_H_
#define _HEADER_UTILS_H_

#include <string.h>
#include <ctype.h>

char* filename_to_header(char* fname) {
	char* r = strdup(fname); 
	int i;
	*r = toupper(*r);
	if(!isalpha(*r)) {
		*r = '_';
	}
	
	for(i = 1; *(r + i) != '\0'; ++i) {
		if(*(r + i) == '.') {
			*(r + i) = '\0';
			break; // leaky
		} else if(!isalnum(*(r + i))) {
			*(r + i) = '_';
		} else {
			*(r + i) = toupper(*(r + i));
		}
	}
	return r;
}

#endif
