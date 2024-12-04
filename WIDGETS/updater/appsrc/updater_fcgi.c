#define _POSIX_C_SOURCE 200809

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "fcgiapp.h"
#include "filesystem.h"

int process_request(FCGX_Request * request) {
	char * request_method = FCGX_GetParam("REQUEST_METHOD", request->envp);
	FCGX_FPrintF(request->out,
			"Status: 200\r\n"
			"Content-Type: text/html\r\n"
			"\r\n"
			"<!DOCTYPE html><html>\r\n"
			"<head></head>\r\n"
			"<body>\r\n"
			"<h1>From FastCGI!</h1>\r\n"
			"</body>\r\n"
			"</html>");
	FCGX_FFlush(request->out);
	FCGX_Finish_r(request);
	return 0;
}

int main(int argc, char *argv[]) {
	FCGX_Request * request;
	int rc, res = 1;

	FCGX_Init();
	request = (FCGX_Request *)malloc(sizeof(*request));
	if (request == NULL) {
		return 1;
	}
	rc = FCGX_InitRequest(request, 0, 0);
	if (rc != 0) {
		fprintf(stderr, "Error: FCGX_InitRequest() %d\n", rc);
		goto error;
	}
	for (;;) {
		rc = FCGX_Accept_r(request);
		if (rc == 0) {
			process_request(request);
		} else {
			fprintf(stderr, "Error: FCGX_Accept_r() %d\n", rc);
			goto error;
		}
	}
	res = 0;
error:
	free(request);
	return res;
}
