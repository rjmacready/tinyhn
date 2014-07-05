
#include "http.h"

void parse_request(struct request *req, 
									 char* buffer,
									 size_t sz) {
	int i = 0;
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
		if(place >= 64) {
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

	fprintf(stderr, "%d headers\n", place);

}
