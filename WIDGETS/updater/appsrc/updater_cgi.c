#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "filesystem.h"
#include "timefn.h"
#include "dynstring.h"

int main(int argc, char *argv[]) {
	dynstring_context_t html_string = DYNSTRING_DEFAULT_INIT;
	char current_date_time[20];
	int64_t unixtime_millis;
	char * replace_location;
	const char * string_to_replace = "REPLACEME";
	char * replace_with;
	int res;

	fprintf(stdout, "Content-Type: text/html\r\n\r\n");

	res = filesystem_loadfiletoram("index.template.html", (void **)&html_string.buf, &html_string.pos);
	if (res != 0) {
		fprintf(stdout, "Could not load template file");
		goto error;
	}
	html_string.size = html_string.pos;
	unixtime_millis = timefn_getcurrentunixtimemillis();
	timefn_formattimefrommillis(current_date_time, unixtime_millis);

	replace_location = strstr(html_string.buf, string_to_replace);
	if (replace_location == NULL) {
		goto error;
	}
	size_t offs = replace_location - html_string.buf;
	replace_with = current_date_time;
	dynstring_splicestring(&html_string, offs, strlen(string_to_replace), replace_with, strlen(replace_with));

	fflush(stdout);
	write(STDOUT_FILENO, html_string.buf, html_string.pos);
	dynstring_free(&html_string);

	return 0;
error:
	return 1;
}

