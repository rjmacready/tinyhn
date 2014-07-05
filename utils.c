
#include "utils.h"

void printfromto(char* buf, int offset, int end) {
	while(offset < end) {
		fprintf(stderr, "%c", buf[offset++]);
	}
}

int match_and_extract_resource(char* buf, 
															 struct bufof *bufo, 
															 struct resource *out) {
	// match /(letter)/xxxxxxxx
	size_t buflen = bufo->end - bufo->offset;
	char* buf_o = buf + bufo->offset;
	if(buflen != 1 + 1 + 1 + 8)  { // '/' letter '/' 8 digits
		return 1;
	}
	
	if(*buf_o != '/') {
		return 1;
	}

	if(*(buf_o + 2) != '/') {
		return 1;
	}
	
	char type = *(buf_o + 1);
	
	if(type != 'u' && type != 'l') {
		return 1;
	}
	
	char id_b[9] = {0};
	sscanf(buf_o + 3, "%8[0-9]", id_b);
	long long id = strtoll(id_b, NULL, 10); // &(id_b + id_r)
	
	fprintf(stderr, "%c %lld\n", type, id);
	
	out->type = type;
	out->id = id;
	
	return 0;
}

int match(char* str, char* buf, struct bufof bufo) {
	size_t sz = strlen(str);
	size_t buflen = bufo.end - bufo.offset;
	if(sz != buflen) {
		return -1;
	}
	return strncmp(str, buf + bufo.offset, sz);
}

char* bufoftos(char* buf, struct bufof* bufo) {
	return strndup(buf + bufo->offset, bufo->end - bufo->offset);
}
