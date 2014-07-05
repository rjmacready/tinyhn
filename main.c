#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "main.h"
#include "response.h"

// generated
#include "index.h"
#include "index2.h"
#include "user.h"

// globals per request
struct request req;
struct resource req_res;

char* get_title_USER() {
	return "this is a dynamic page";
}

char* get_user_id_USER() {
	char* r = (char*)malloc(sizeof(char) * 200);
	sprintf(r, "%lld", req_res.id);
	return r;
}

int main(int argc, char** argv) {
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr addr;
	socklen_t addrlen;
	char* buffer;
	buffer = (char*)malloc(sizeof(char) * BUF_SIZE);
	
	addrlen = sizeof(addr);
	
	int s;
	memset(&hints, 0, sizeof(hints));
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if(0 != getaddrinfo(NULL, "8080", &hints, &res)) {
		fprintf(stderr, "Error on getaddrinfo\n");
		exit(1);
	}
	
	//	if(1 != inet_pton(AF_INET, "0.0.0.0", &(sa.sin_addr))) {
	//		fprintf(stderr, "Error on inet_pton\n");
	//		return 1;
	//	}
 
	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(-1 == s) {
		fprintf(stderr, "Error on socket\n");
		exit(2);
	}
	
	int yes = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	int port = bind(s, res->ai_addr, res->ai_addrlen);
	if(-1 == port) {
		fprintf(stderr, "Error on bind, errno = %d\n", errno);
		exit(3);
	}
	
	if(-1 == listen(s, 5)) {
		fprintf(stderr, "Error on listen, errno = %d\n", errno);
		exit(4);
	}

	int cs;

 _accept:

	cs = accept(s, &addr, &addrlen);
	if(-1 == cs) {
		fprintf(stderr, "Error on accept, errno = %d\n", errno);
		// TODO handle error?
		goto _accept;
	}

	pid_t cpid = fork();
	
	if(cpid > 0 || cpid == -1) {
		// this is the parent process
		// TODO maybe we want to handle a failure to spawn
		goto _accept;
	}
	
	pid_t pid = getpid();
	int sz;
	
 _request:
	
	sz = recv(cs, buffer, BUF_SIZE, 0);
	if(sz == BUF_SIZE) {
		fprintf(stderr, "Request was too big on %d\n", pid);
		exit(6);
	}

	if(sz == 0) {
		fprintf(stderr, "Got nothing on %d (Connection closed, probably)", pid);
		exit(7);
	}

	if(sz == -1) {
		fprintf(stderr, "recv returned -1 on %d, %s", pid, strerror(errno));
		exit(7);
	}

	fprintf(stderr, "Got %d bytes of data on %d\n", sz, pid);
	
	int i = 0;
	FILE* fcs = fdopen(cs, "w");

	req.method.offset = 0;
	while(++i < sz && buffer[i] != ' ');	
	req.method.end = i;
	
	printfromto(buffer, 
							req.method.offset, 
							req.method.end);
	fprintf(stderr, " ");

	while(++i < sz && buffer[i] == ' ');	
	req.resource.offset = i;
	while(++i < sz && buffer[i] != ' ');	
	req.resource.end = i; 

	printfromto(buffer, 
							req.resource.offset, 
							req.resource.end);
	fprintf(stderr, " ");
	
	while(++i < sz && buffer[i] == ' ');	
	req.http.offset = i;
	while(++i < sz && buffer[i] != '\r');	
	req.http.end = i; 

	//fprintf(stderr, "Http from %d to %d\n", req.http.offset, req.http.end);
	printfromto(buffer, 
							req.http.offset, 
							req.http.end);
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
		
		req.headers[place].offset = i;
		
		// read until there's :
		while(++i < sz) {
			if(buffer[i] == ':') {
				req.headers[place].end = i;
				break;
			}
		}
		
		//fprintf(stderr, "H: %d - %d\n", 
		//				req.headers[place].offset, 
		//				req.headers[place].end);

		printfromto(buffer, 
								req.headers[place].offset, 
								req.headers[place].end);

		// read spaces
		while(++i < sz && buffer[i] == ' ');
		++place;
		req.headers[place].offset = i;
		
		// read value
		while(++i < sz) {
			if(buffer[i] == '\r') {
				req.headers[place].end = i;
				break;
			}
		}
		
		//fprintf(stderr, "V: %d - %d\n", 
		//				req.headers[place].offset, 
		//				req.headers[place].end);
		fprintf(stderr, ": ");
		printfromto(buffer, 
								req.headers[place].offset, 
								req.headers[place].end);
		fprintf(stderr, "\n");
		// read \r\n
		++i;
		++i;

		++place;
	} while(i < sz);
	
	fprintf(stderr, "%d headers\n", place);
	
	{
		// routing tests
		// /            (dynamic)
		// /u?id=xxx (dynamic)
		// /l?id=xxx (dynamic)
		
		// static files

		// else, 404
		
		if(0 == match("/", buffer, req.resource)) {
			// static 
			
			char* c0 = "HTTP/1.1 200 OK\r\n";
			char* c1 = "Content-Type: text/html; charset=us-ascii\r\n";
			char* c2 = malloc(sizeof(char) * 30);
			snprintf(c2, sizeof(char) * 30, "Content-Length: %d\r\n", strlen(RSRC_INDEX));
	
			send(cs, c0, strlen(c0), 0);
			send(cs, c1, strlen(c1), 0);
			send(cs, c2, strlen(c2), 0);

			send(cs, "\r\n", 2, 0);
			send(cs, RSRC_INDEX, strlen(RSRC_INDEX), 0);
	
		} else if(0 == match_and_extract_resource(buffer, req.resource, &req_res)) {						
			if(NULL == fcs) {
				fprintf(stderr, "Error on fdopen\n");
				exit(8);
			}
			
			
			fprintf(fcs, "HTTP/1.1 200 OK\r\n");
			fprintf(fcs, "Content-Type: text/html; charset=us-ascii\r\n");
			struct resp_string* resp; int resp_no;
			
			int written = resource_USER(&resp, &resp_no);
			fprintf(fcs, "Content-Length: %d\r\n\r\n", written);
			
			for(i = 0; i < resp_no; ++i) {
				fprintf(fcs, "%s", resp[i].buffer);
			}
			fflush(fcs);
			
		} else {
			// 404, static response
			
			char* c0 = "HTTP/1.1 404 Not Found\r\n";
			char* c1 = "Content-Type: text/html; charset=us-ascii\r\n";
			char* c2 = malloc(sizeof(char) * 30);
			snprintf(c2, sizeof(char) * 30, "Content-Length: %d\r\n", strlen(RSRC_INDEX));
	
			send(cs, c0, strlen(c0), 0);
			send(cs, c1, strlen(c1), 0);
			send(cs, c2, strlen(c2), 0);

			send(cs, "\r\n", 2, 0);
			send(cs, RSRC_INDEX2, strlen(RSRC_INDEX2), 0);
			
		}
	}

	goto _request;
 
	close(cs);
		
	return 0;
}
