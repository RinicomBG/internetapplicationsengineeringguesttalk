#ifndef DYNSTRING_H
#define DYNSTRING_H

/* Disable unknown pragma warnings for MSVC++ */
/* Also available in Project Settings->C/C++->Advanced "Disable Specific Warnings" */
#pragma warning (disable : 4068)

#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>

#include <malloc.h>
#include <string.h>
#ifdef __cplusplus
#include <string>
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
typedef struct dynstring_context dynstring_context_t;

struct dynstring_context {
	char * buf;
	size_t size;
	size_t pos;
};

#define DYNSTRING_DEFAULT_INIT { NULL, 0, 0 }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static int dynstring_initialisezero(dynstring_context_t * ctx) {
	ctx->buf = NULL;
	ctx->size = 0;
	ctx->pos = 0;
	return 0;
}

static int dynstring_initialise(dynstring_context_t * ctx, size_t size) {
	ctx->buf = (char *)malloc(size);
	if (ctx->buf == NULL) {
		return -1;
	}
	ctx->size = size;
	ctx->pos = 0;
	return 0;
}

static int dynstring_set(dynstring_context_t * ctx, char * string, size_t pos, size_t size) {
	ctx->buf = string;
	ctx->pos = pos;
	ctx->size = size;
	return 0;
}

static int dynstring_appendchar(dynstring_context_t * ctx, int c) {
	if (ctx->pos >= ctx->size) {
		char * newbuf = (char *)realloc(ctx->buf, ctx->size + 64);
		if (newbuf == NULL) {
			return -1;
		}
		ctx->buf = newbuf;
		ctx->size = ctx->size + 64;
	}
	ctx->buf[ctx->pos++] = c;
	return 0;
}

static int dynstring_extend(dynstring_context_t * ctx, size_t extend_by);

static int dynstring_insertstring(dynstring_context_t * ctx, size_t insert_at, const char * str, size_t str_len) {
	if (insert_at > ctx->pos) {
		return -1; // Invalid position
	}
	if (str_len == 0) {
		return 0;
	}
	if (str_len + ctx->pos >= ctx->size) {
		int rc = dynstring_extend(ctx, str_len);
		if (rc != 0) {
			return rc;
		}
	}
	memmove(ctx->buf + insert_at + str_len, ctx->buf + insert_at, ctx->pos - insert_at);
	memcpy(ctx->buf + insert_at, str, str_len);
	ctx->pos += str_len;
	return 0;
}

static int dynstring_splicestring(dynstring_context_t *ctx, size_t insert_at, size_t replace_len, const char *str, size_t str_len) {
	if (insert_at > ctx->pos) {
		return -1; // Invalid position
	}
	if (str_len == 0 && replace_len == 0) {
		return 0;
	}

	// Calculate the size adjustment needed (if any)
	size_t end_of_replace = insert_at + replace_len;
	size_t new_len = ctx->pos + str_len - replace_len;

	if (new_len >= ctx->size) {
		int rc = dynstring_extend(ctx, new_len - ctx->pos);
		if (rc != 0) {
			return rc;
		}
	}
	if (str_len != replace_len) {
		memmove(ctx->buf + insert_at + str_len, ctx->buf + insert_at + replace_len, ctx->pos - end_of_replace);
	}
	memcpy(ctx->buf + insert_at, str, str_len);
	ctx->pos = new_len;
	return 0;
}

static int dynstring_appendstring(dynstring_context_t * ctx, const char * str, size_t str_len) {
	if (str_len == 0) {
		return 0;
	}
	if (str_len + ctx->pos >= ctx->size) {
		int rc = dynstring_extend(ctx, str_len);
		if (rc != 0) {
			return rc;
		}
	}
	memcpy(ctx->buf + ctx->pos, str, str_len);
	ctx->pos += str_len;
	return 0;
}

static int dynstring_appendstringz_va(dynstring_context_t * ctx, va_list args) {
	char * str;
	size_t str_len;
	int rc;
	str = va_arg(args, char *);
	while (str != NULL) {
		str_len = strlen(str);
		rc = dynstring_appendstring(ctx, str, str_len);
		if (rc) { break; }
		str = va_arg(args, char *);
	}
	return rc;
}

static int dynstring_appendstringz(dynstring_context_t * ctx, ...) {
	int rc;
	va_list args;
	va_start(args, ctx);
	rc = dynstring_appendstringz_va(ctx, args);
	va_end(args);
	return rc;
}

static int dynstring_endswithchar(dynstring_context_t * ctx, int c) {
	return ctx->buf[ctx->pos - 1] == c;
}

static size_t dynstring_length(dynstring_context_t * ctx) {
	return ctx->pos;
}

static void dynstring_setlength(dynstring_context_t * ctx, size_t len) {
	ctx->pos = len;
}

static void dynstring_empty(dynstring_context_t * ctx) {
	ctx->pos = 0;
	if (ctx->buf) {
		ctx->buf[ctx->pos] = '\0';
	}
}

static void dynstring_free(dynstring_context_t * ctx) {
	if (ctx->buf) {
		free(ctx->buf);
		ctx->buf = NULL;
	}
	ctx->pos = 0;
	ctx->size = 0;
}

static int dynstring_compact(dynstring_context_t * ctx) {
	char * newbuf;
	size_t newsize;
	newsize = ctx->pos + 1;
	newbuf = (char *)realloc(ctx->buf, newsize);
	if (newbuf) {
		ctx->buf = newbuf;
		ctx->size = newsize;
		return 0;
	} else {
		return -1;
	}
}

static size_t dynstring_freespace(dynstring_context_t * ctx) {
	return ctx->size - ctx->pos;
}

static int dynstring_erase(dynstring_context_t * ctx, size_t start, size_t len) {
	size_t end = start + len;
	memmove(ctx->buf + start, ctx->buf + end, ctx->pos - end);
	ctx->pos -= len;
	return 0;
}

static int dynstring_extend(dynstring_context_t * ctx, size_t extend_by) {
	char * newbuf = (char *)realloc(ctx->buf, ctx->size + extend_by);
	if (newbuf == NULL) {
		return -1;
	}
	ctx->buf = newbuf;
	ctx->size = ctx->size + extend_by;
	return 0;
}

static char * dynstring_getcstring(dynstring_context_t * ctx) {
	if (ctx->size > ctx->pos) { // && ctx->buf[ctx->pos] == '\0') {
		ctx->buf[ctx->pos] = '\0';
		return ctx->buf;
	}
	dynstring_appendchar(ctx, '\0');
	ctx->pos --;
	return ctx->buf;
}

static char * dynstring_detachcstring(dynstring_context_t * ctx) {
	char * rawstring;
	dynstring_appendchar(ctx, '\0');
	dynstring_compact(ctx);
	rawstring = ctx->buf;
	ctx->buf = NULL;
	dynstring_free(ctx);
	return rawstring;
}

#ifdef __cplusplus

static std::string dynstring_tostring(dynstring_context_t * dynstr) {
	std::string res(dynstring_getcstring(dynstr));
	dynstring_free(dynstr);
	return res;
}

#endif /* __cplusplus */

#pragma GCC diagnostic pop

#endif /* DYNSTRING_H */


