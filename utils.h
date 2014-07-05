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

struct request {
	struct bufof method;
	struct bufof resource;
	struct bufof http;
	struct bufof headers[REQ_SIZE];
	struct bufof content;
};

#define BUF_SIZE 2048

struct resource {
	char type;
	long long id;
};


void printfromto(char* buf, int offset, int end);
int match_and_extract_resource(char* buf, 
															 struct bufof bufo, 
															 struct resource *out);
int match(char* str, char* buf, struct bufof bufo);

#endif

