#ifndef __TIMEFN_H__
#define __TIMEFN_H__

#include <stdint.h>

#ifdef __cplusplus
#include <string>

extern "C" {
#endif

#define TIMEFN_DATETIMESTRINGSIZE 20
#define TIMEFN_ONEDAYINMILLIS (24L*60L*60L*1000L)

/* Changing this struct will have bad effects on datfile.c, please be careful! */
/* SEARCH for timefntime_t * */
typedef struct {
	uint16_t year, month, day, hour, minute, second;
} timefntime_t;

int64_t timefn_getunixmillis(int year, int month, int day, int hour, int minute, int second);
void timefn_formattimefromdatetimestruct_inplace(char * dst, timefntime_t * datetimestruct);
int64_t timefn_getunixmillisfromtimestruct(timefntime_t * timestruct);
void timefn_gettimefromunixtimemillis(timefntime_t * timeoutput, int64_t unixtimemillis);
char * timefn_formattimefromdatetimestruct(timefntime_t * datetimestruct);
int64_t timefn_getcurrentunixtimemillis();
int64_t timefn_getcurrentunixtime();
void timefn_getcurrenttimedatestruct(timefntime_t * timeoutput);
void timefn_formattimefrommillis(char * dst, int64_t unixtimemillis);
int64_t timefn_parsetimetomillis(const char * src);

#ifdef __cplusplus
}

#pragma warning (disable : 4068)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static std::string timefn_formattimefromunixtime_str(int64_t unix_time) {
	std::string buf;
	buf.resize(20);
	timefn_formattimefrommillis((char *)buf.data(), unix_time * 1000);
	buf.resize(19);
	return buf;
}
#pragma GCC diagnostic pop
#endif

#endif /* __TIMEFN_H__ */
