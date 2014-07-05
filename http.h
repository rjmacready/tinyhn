#ifndef _HTTP_H_
#define _HTTP_H_

#include "utils.h"

struct bufof* get_header_value(struct request *req, 
															 char* header);

void parse_request(struct request *req, 
									 char* buffer,
									 size_t sz);

#endif
