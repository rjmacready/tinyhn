#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "http.h"
#include "utils.h"

// generated
#include "configs.h"
#include "index.h"
#include "index2.h"
#include "newuser.h"
#include "user.h"

// globals per request
struct request req;
struct resource req_res;
struct action req_act;

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
	
	if(0 != getaddrinfo(NULL, CONFIG_port, &hints, &res)) {
		fprintf(stderr, "Error on getaddrinfo\n");
		exit(1);
	}

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
	if(NULL == fcs) {
		fprintf(stderr, "Error on fdopen\n");
		exit(8);
	}

	parse_request(&req, buffer, sz);	

	{
		// routing tests
		// /            (dynamic)
		// /u?id=xxx (dynamic)
		// /l?id=xxx (dynamic)
		
		// static files

		// else, 404
		
		if(0 == match("/", buffer, req.resource)) {
			fprintf(fcs, "HTTP/1.1 200 OK\r\n");
			fprintf(fcs, "Content-Type: text/html; charset=us-ascii\r\n");
			struct resp_string* resp; int resp_no;
			
			int written = resource_INDEX(&resp, &resp_no);
			fprintf(fcs, "Content-Length: %d\r\n\r\n", written);
			
			for(i = 0; i < resp_no; ++i) {
				fprintf(fcs, "%s", resp[i].buffer);
			}
			fflush(fcs);

		} else if(0 == match("/n", buffer, req.resource)) {

			fprintf(fcs, "HTTP/1.1 200 OK\r\n");
			fprintf(fcs, "Content-Type: text/html; charset=us-ascii\r\n");
			struct resp_string* resp; int resp_no;
			
			int written = resource_NEWUSER(&resp, &resp_no);
			fprintf(fcs, "Content-Length: %d\r\n\r\n", written);
			
			for(i = 0; i < resp_no; ++i) {
				fprintf(fcs, "%s", resp[i].buffer);
			}
			fflush(fcs);
	
		} else if(0 == match_resource(buffer, &req.resource, &req_res)) {						
			
			fprintf(fcs, "HTTP/1.1 200 OK\r\n");
			fprintf(fcs, "Content-Type: text/html; charset=us-ascii\r\n");
			struct resp_string* resp; int resp_no;
			
			int written = resource_USER(&resp, &resp_no);
			fprintf(fcs, "Content-Length: %d\r\n\r\n", written);
			
			for(i = 0; i < resp_no; ++i) {
				fprintf(fcs, "%s", resp[i].buffer);
			}
			fflush(fcs);
			
		}else if(0 == match_action(buffer, &req.resource, &req_act)) {						

			// redirect

			fprintf(fcs, "HTTP/1.1 302 Found\r\n");
			fprintf(fcs, "Location: /\r\n\r\n");
			fflush(fcs);

		} else {
			fprintf(fcs, "HTTP/1.1 404 Not Found\r\n");
			fprintf(fcs, "Content-Type: text/html; charset=us-ascii\r\n");
			struct resp_string* resp; int resp_no;
			
			int written = resource_INDEX2(&resp, &resp_no);
			fprintf(fcs, "Content-Length: %d\r\n\r\n", written);
			
			for(i = 0; i < resp_no; ++i) {
				fprintf(fcs, "%s", resp[i].buffer);
			}
			fflush(fcs);
		}
	}

	goto _request;
 
	close(cs);
		
	return 0;
}
