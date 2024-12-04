#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILESYSTEM_PATH_MAXLEN
#define FILESYSTEM_PATH_MAXLEN 4096
#endif

#define FILESYSTEM_ATTR_DIRECTORY 8
#define FILESYSTEM_ATTR_SYMBOLICLINK 0x10
#define FILESYSTEM_ATTR_FILE 1
#define FILESYSTEM_ATTR_EXECUTABLE 2

struct filesystem_fileitem {
	char * name;
	uint32_t attrs;
};

int filesystem_loadfiletoram(const char * path, void ** data_ptr, size_t * data_sz_ptr);
int filesystem_saveramtofile(const char * path, const void * data, const size_t size);
int filesystem_fileexists(const char * path);
int filesystem_deletefile(const char * path);
int filesystem_movefile(const char * dst_filename, const char * src_filename);

typedef int (*filesystem_callback)(void * user_data, const char * path);
int filesystem_recursedirectories(void * userdata, filesystem_callback cb, const char * path, int depth);

#ifdef __cplusplus
}

#include <string>
#include <functional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static std::string filesystem_loadfiletostring(const char * path) {
	char * data;
	size_t data_sz;
	int rc;
	rc = filesystem_loadfiletoram(path, (void **)&data, &data_sz);
	if (rc == 0) {
		std::string res = std::string((char *)data, data_sz);
		free(data);
		return res;
	}
	return "";
}

static int filesystem_recursedirectories_lambda_callback(void * userdata, const char * path) {
	std::function<int(const char * filename)> * cb = (std::function<int(const char * filename)> *)userdata;
	return (*cb)(path);
}
static int filesystem_recursedirectories_lambda(std::function<int(const char * filename)> cb, const char * path, int depth) {
	return filesystem_recursedirectories(&cb, filesystem_recursedirectories_lambda_callback, path, depth);
}

#pragma GCC diagnostic pop

#endif

#endif /* FILESYSTEM_H */
