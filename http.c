
#include "http.h"

inline struct bufof* get_dictionary_value(char* buffer,
																	 struct bufof *dict,
																	 int no,
																	 char* key) {
	int i = 0;
	for(; i < no; i += 2) {
		if(0 == match(key, buffer, dict[i])) {
			return &(dict[i + 1]);
		}
	}
	return NULL;
}

struct bufof* get_post_value(struct request *req, 
															 char* header) {
	return get_dictionary_value(req->buffer, req->content_data.post_data, 
															req->content_data.post_no, header);
}

struct bufof* get_header_value(struct request *req, 
															 char* header) {
	return get_dictionary_value(req->buffer, req->headers, 
															req->headers_no, header);
}

void parse_post_data(struct request *req, 
										 char* buffer) {

	memset(req, 0, sizeof(struct request));
	
	int place = 0, i = req->content.offset;
	int sz = req->content.end;
	
	do {
		if(place >= POST_SIZE) {
			fprintf(stderr, "Post too big");
			exit(8);
		}

		// read key
		req->content_data.post_data[place].offset = i;		
		while(++i < sz && buffer[i] != '=');
		req->content_data.post_data[place].end = i; 

		printfromto(buffer, 
								req->content_data.post_data[place].offset, 
								req->content_data.post_data[place].end);

		++place;

		// read '='
		++i;
		// read value
		req->content_data.post_data[place].offset = i;
		while(++i < sz && buffer[i] != '&');
		req->content_data.post_data[place].end = i; 
		
		fprintf(stderr, ": ");
		printfromto(buffer, 
								req->content_data.post_data[place].offset, 
								req->content_data.post_data[place].end);
		fprintf(stderr, "\n");
		
		++place;
	} while(i < sz);

	req->content_data.post_no = place;
	fprintf(stderr, "Post vars: %d\n", place / 2);
}

void parse_request(struct request *req, 
									 char* buffer,
									 size_t sz) {
	int i = 0;
	req->buffer = buffer;
	req->buffer_sz = sz;
	req->method.offset = 0;
	while(++i < sz && buffer[i] != ' ');	
	req->method.end = i;
	
	printfromto(buffer, 
							req->method.offset, 
							req->method.end);
	fprintf(stderr, " ");

	while(++i < sz && buffer[i] == ' ');	
	req->resource.offset = i;
	while(++i < sz && buffer[i] != ' ');	
	req->resource.end = i; 

	printfromto(buffer, 
							req->resource.offset, 
							req->resource.end);
	fprintf(stderr, " ");
	
	while(++i < sz && buffer[i] == ' ');	
	req->http.offset = i;
	while(++i < sz && buffer[i] != '\r');	
	req->http.end = i; 

	//fprintf(stderr, "Http from %d to %d\n", req->http.offset, req->http.end);
	printfromto(buffer, 
							req->http.offset, 
							req->http.end);
	fprintf(stderr, "\n");
	// consume \r\n
	++i; ++i;
	
	// now we have arbitrary headers
	
	int place = 0;
	do {
		if(place >= REQ_SIZE) {
			fprintf(stderr, "Request with too many headers");
			exit(8);
		}
		
		if(buffer[i] == '\r') {
			break;
		}
		
		req->headers[place].offset = i;
		
		// read until there's :
		while(++i < sz) {
			if(buffer[i] == ':') {
				req->headers[place].end = i;
				break;
			}
		}
		
		//fprintf(stderr, "H: %d - %d\n", 
		//				req->headers[place].offset, 
		//				req->headers[place].end);

		printfromto(buffer, 
								req->headers[place].offset, 
								req->headers[place].end);

		// read spaces
		while(++i < sz && buffer[i] == ' ');
		++place;
		req->headers[place].offset = i;
		
		// read value
		while(++i < sz) {
			if(buffer[i] == '\r') {
				req->headers[place].end = i;
				break;
			}
		}
		
		//fprintf(stderr, "V: %d - %d\n", 
		//				req->headers[place].offset, 
		//				req->headers[place].end);
		fprintf(stderr, ": ");
		printfromto(buffer, 
								req->headers[place].offset, 
								req->headers[place].end);
		fprintf(stderr, "\n");
		// read \r\n
		++i;
		++i;

		++place;
	} while(i < sz);	

	// read \n
	++i;

	req->headers_no = place;
	
	
	struct bufof *content_length = get_header_value(req, "Content-Length");
	int i_content_length = 0;

	if(content_length != NULL) {
		fprintf(stderr, "Content-Length: ");
		printfromto(buffer, 
								content_length->offset, 
								content_length->end);
		fprintf(stderr, "\n");
		char* s_content_length = bufoftos(buffer, content_length);
		{
			fprintf(stderr, " as string: %s\n", s_content_length);
			i_content_length = atoi(s_content_length);
		}
		free(s_content_length);
	}

	req->content.offset = i;
	req->content.end = i + i_content_length;

	fprintf(stderr, "Content: ");
	printfromto(buffer, 
								req->content.offset, 
								req->content.end);
	fprintf(stderr, "\n");
	
	i = sz;
	fprintf(stderr, "i at %d/%d, last header offset at %d\n", 
					i, sz,
					req->headers[place - 1].end);
	fprintf(stderr, "%d headers\n", place / 2);

	
	if(0 == match("POST", buffer, req->method)) {
		parse_post_data(req, buffer);
	}	
}
