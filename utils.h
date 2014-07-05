#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "response.h"

struct bufof {
	int offset;
	int end;
};

#define REQ_SIZE 64
#define POST_SIZE 20

struct request {
	char* buffer;
	size_t buffer_sz;
	
	struct bufof method;
	struct bufof resource;
	struct bufof http;
	int headers_no;
	struct bufof headers[REQ_SIZE];
	struct bufof content;

	char type;
	struct {
		int post_no;
		struct bufof post_data[POST_SIZE];
	} content_data;
};

#define BUF_SIZE 2048

struct resource {
	char type;
	long long id;
};

struct action {
	char type;
	char action;
};

char* bufoftos(char* buf, struct bufof *bufo);
void printfromto(char* buf, int offset, int end);
int match_resource(char* buf, 
									 struct bufof *bufo, 
									 struct resource *out);
int match_action(char* buf, 
									 struct bufof *bufo, 
									 struct action *out);
int match(char* str, char* buf, struct bufof bufo);

#endif

