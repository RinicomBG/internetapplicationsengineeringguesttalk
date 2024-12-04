#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "timefn.h"
#include "filesystem.h"
#include "dynstring.h"

sig_atomic_t quit_programme;

void sighandler_sigint(int signo, siginfo_t *info, void *context) {
	quit_programme = 1;
}

int update_html_file(const char * string_to_replace, const char * replace_with) {
	char * html_data;
	size_t html_data_size;
	dynstring_context_t html_string = DYNSTRING_DEFAULT_INIT;
	char * replace_location;
	int res;
	res = filesystem_loadfiletoram("index.template.json", (void **)&html_data, &html_data_size);
	if (res != 0) {
		goto error;
	}
	dynstring_set(&html_string, html_data, html_data_size, html_data_size);
	html_data = NULL;
	html_data_size = 0;
	replace_location = strstr(html_string.buf, string_to_replace);
	if (replace_location == NULL) {
		goto error;
	}
	size_t offs = replace_location - html_string.buf;
	dynstring_splicestring(&html_string, offs, strlen(string_to_replace), replace_with, strlen(replace_with));
	res = filesystem_saveramtofile("index.json", html_string.buf, html_string.pos);
	if (res != 0) {
		goto error;
	}
error:
	dynstring_free(&html_string);
	return -1;
}

int main(int argc, char *argv[]) {
	int res;
	struct sigaction newaction;
	struct sigaction oldaction;
	timefntime_t timenow;
	quit_programme = 0;
	fprintf(stdout, "File Updater, Running...\n");
	memset(&newaction, 0, sizeof(newaction));
	newaction.sa_flags = SA_SIGINFO;
	newaction.sa_sigaction = &sighandler_sigint;
	res = sigaction(SIGINT, &newaction, &oldaction);
	while (!quit_programme) {
		char formatteddatetime[20];
		timefn_getcurrenttimedatestruct(&timenow);
		timefn_formattimefromdatetimestruct_inplace(formatteddatetime, &timenow);
		errno = 0;
		update_html_file("REPLACEME", formatteddatetime);
		if (res) {
			fprintf(stdout, "%d: %s\n", res, strerror(errno));
		}
		res = sleep(5);
	}
	fprintf(stdout, "Terminating.\n");
	return 0;
}
