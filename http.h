#ifndef _HTTP_H_
#define _HTTP_H_

#include "utils.h"

void parse_request(struct request *req, 
									 char* buffer,
									 size_t sz);

#endif
