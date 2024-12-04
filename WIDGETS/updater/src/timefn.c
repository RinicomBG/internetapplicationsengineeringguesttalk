#define USE_UTILITY_FN
#define USE_HOMEGROWN_TM_TO_EPOCH

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define __COMPILE_FOR_WIN32
#endif

#ifdef __COMPILE_FOR_LINUX

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 201112L
#endif /* _POSIX_C_SOURCE */

// Get the definition for timegm included!
#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <time.h>
#include <math.h>

#endif /* __COMPILE_FOR_LINUX */

#include <time.h>
#include "timefn.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// #include "leakdetector.h"

#ifdef __COMPILE_FOR_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


// #define USE_UTILITY_FN
#ifdef USE_UTILITY_FN
#include "utilityfn.h"
#include <string.h>
#endif

#ifdef __COMPILE_FOR_LINUX
#include <stdlib.h>
#ifndef USE_HOMEGROWN_TM_TO_EPOCH
// can use this to avoid usage of timegm() see man page
// seems that the setenv nonesense does not work well
// for now just use timegm (will this work in Alpine?)
static time_t my_timegm(struct tm * tm) {
	/*
	time_t ret;
	char *tz;
	tz = getenv("TZ");
	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	if (tz) {
		setenv("TZ", tz, 1);
	} else {
		unsetenv("TZ");
		tzset();
	}
	return ret;
	*/

	return timegm(tm);
}
#endif
#endif

#ifdef USE_HOMEGROWN_TM_TO_EPOCH
/*
 * Yes, I hate re-inventing the wheel too but have you looked at the functions available
 * in the C library for performing conversions like this?
 * At least this is an implementation of a well used an well known algorithm.
 * - Benjamin Green 2021-02-03
 * 
 * The algorithm here is Robert Tantzen's Algorithm 199
 * "Algorithm 199: Conversions Between Calendar Date and Julian Day Number", Communications of the ACM, Vol. 8, August 1963, p444
 * https://howardhinnant.github.io/date_algorithms.html
 * https://gist.github.com/cjheath/3141204
 * https://www.clarusft.com/elegant-inelegance-algorithm-199/
 * https://www.digm.com/Resources/Gregory/Date-Manipulations-in-WFL.pdf
 * https://scriptinghelpers.org/questions/25121/how-do-you-get-the-date-and-time-from-unix-epoch-time
 * https://www.oryx-embedded.com/doc/date__time_8c_source.html
 */
static time_t my_timegm(struct tm * tm) {
	int64_t y, m, d;
	int64_t t;
	y = tm->tm_year + 1900;
	m = tm->tm_mon + 1;
	d = tm->tm_mday;

	if (m <= 2) {
		m += 12;
		y -= 1;
	}

	t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
	t += (30 * m) + (3 * (m + 1) / 5) + d;
	t -= 719561;
	t *= 86400;
	t += (3600 * tm->tm_hour) + (60 * tm->tm_min) + tm->tm_sec;

	return t;
}
#endif

int64_t timefn_getunixmillis(int year, int month, int day, int hour, int minute, int second) {
#ifdef __COMPILE_FOR_WIN32
	FILETIME filetime;
	SYSTEMTIME systemtime;
	ULARGE_INTEGER resultingtime;

	systemtime.wDayOfWeek = 0;
	systemtime.wYear = year;
	systemtime.wMonth = month;
	systemtime.wDay = day;
	systemtime.wHour = hour;
	systemtime.wMinute = minute;
	systemtime.wSecond = second;
	systemtime.wMilliseconds = 0;
	SystemTimeToFileTime(&systemtime, &filetime);

	resultingtime.HighPart = filetime.dwHighDateTime;
	resultingtime.LowPart = filetime.dwLowDateTime;

	// remove the number of 100 nanosecond intervals between 1601 and 1970
	resultingtime.QuadPart = resultingtime.QuadPart - (11644473600000L * 10000L);
	// convert the 100 nanosecond intervals into millis
	resultingtime.QuadPart = resultingtime.QuadPart / 10000L;

	return (uint64_t)resultingtime.QuadPart;
#endif
#ifdef __COMPILE_FOR_LINUX
	struct tm tm;
	time_t unix_time;
	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = second;
	unix_time = my_timegm(&tm);
	if (unix_time == -1) { unix_time = 0; }
	return (unix_time * 1000);
#endif
}

void timefn_gettimefromunixtimemillis(timefntime_t * timeoutput, int64_t unixtimemillis) {
#ifdef __COMPILE_FOR_WIN32
	FILETIME filetime;
	SYSTEMTIME systemtime;
	ULARGE_INTEGER inputtime;

	inputtime.QuadPart = unixtimemillis * 10000L;
	inputtime.QuadPart += (11644473600000L * 10000L);
	filetime.dwHighDateTime = inputtime.HighPart;
	filetime.dwLowDateTime = inputtime.LowPart;
	FileTimeToSystemTime(&filetime, &systemtime);

	timeoutput->year = systemtime.wYear;
	timeoutput->month = systemtime.wMonth;
	timeoutput->day = systemtime.wDay;
	timeoutput->hour = systemtime.wHour;
	timeoutput->minute = systemtime.wMinute;
	timeoutput->second = systemtime.wSecond;
#else
	struct tm tm;
	time_t unix_time;
	unix_time = unixtimemillis / 1000;
	gmtime_r(&unix_time, &tm);
	timeoutput->year = tm.tm_year + 1900;
	timeoutput->month = tm.tm_mon + 1;
	timeoutput->day = tm.tm_mday;
	timeoutput->hour = tm.tm_hour;
	timeoutput->minute = tm.tm_min;
	timeoutput->second = tm.tm_sec;
#endif
}

void timefn_formattimefrommillis(char * dst, int64_t unixtimemillis) {
	timefntime_t timebuf;
	timefn_gettimefromunixtimemillis(&timebuf, unixtimemillis);
	timefn_formattimefromdatetimestruct_inplace(dst, &timebuf);
}

/*
 * 1970-01-01T00:00:00
 *           111111111
 * 0123456789012345678
 * 19 characters without null terminator
 * 20 characters with null terminator
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation="
void timefn_formattimefromdatetimestruct_inplace(char * dst, timefntime_t * datetimestruct) {
	snprintf(dst, 20, "%04d-%02d-%02dT%02d:%02d:%02d",
			datetimestruct->year, datetimestruct->month, datetimestruct->day,
			datetimestruct->hour, datetimestruct->minute, datetimestruct->second);
}
char * timefn_formattimefromdatetimestruct(timefntime_t * datetimestruct) {
	/* Simple helper, min size of dst is 19 bytes + 1 for null trm */
	char * dst = malloc(20);
	timefn_formattimefromdatetimestruct_inplace(dst, datetimestruct);
	return dst;
}
#pragma GCC diagnostic pop

int64_t timefn_getunixmillisfromtimestruct(timefntime_t * timestruct) {
	return timefn_getunixmillis(timestruct->year, timestruct->month, timestruct->day, timestruct->hour, timestruct->minute, timestruct->second);
}

int64_t timefn_getcurrentunixtimemillis() {
#ifdef __COMPILE_FOR_WIN32
	SYSTEMTIME systemtime;
	GetSystemTime(&systemtime);
	return timefn_getunixmillis(systemtime.wYear, systemtime.wMonth, systemtime.wDay, systemtime.wHour, systemtime.wMinute, systemtime.wSecond) + systemtime.wMilliseconds;
#else
	long ms;
	time_t s;
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	s = spec.tv_sec;
	ms = round(spec.tv_nsec / 1.0e6); // nano seconds to milliseconds
	if (ms > 999) {
		s ++;
		ms = 0;
	}
	return (s * 1000) + ms;
#endif
}

int64_t timefn_getcurrentunixtime() {
#ifdef __COMPILE_FOR_WIN32
	SYSTEMTIME systemtime;
	GetSystemTime(&systemtime);
	return timefn_getunixmillis(systemtime.wYear, systemtime.wMonth, systemtime.wDay, systemtime.wHour, systemtime.wMinute, systemtime.wSecond) / 1000LL;
#else
	time_t s;
	s = time(NULL);
	return (int64_t)s;
#endif
}

void timefn_getcurrenttimedatestruct(timefntime_t * timeoutput) {
#ifdef __COMPILE_FOR_WIN32
	SYSTEMTIME systemtime;
	GetSystemTime(&systemtime);
	timeoutput->year = systemtime.wYear;
	timeoutput->month = systemtime.wMonth;
	timeoutput->day = systemtime.wDay;
	timeoutput->hour = systemtime.wHour;
	timeoutput->minute = systemtime.wMinute;
	timeoutput->second = systemtime.wSecond;
#else
	timefn_gettimefromunixtimemillis(timeoutput, timefn_getcurrentunixtimemillis());
#endif
}

int64_t timefn_parsetimetomillis(const char * src) {
	int year, month, day, hour, min, sec;
	int sep = 0;
#ifdef USE_UTILITY_FN
	// If you want to know what the separator is...
	if (strlen(src) < 19) {
		return 0;
	}
	year = Utility_aToInt(src);
	month = Utility_aToInt(src + 5);
	day = Utility_aToInt(src + 5 + 3);
	sep = *(src + 5 + 3 + 2);
	hour = Utility_aToInt(src + 5 + 3 + 3);
	min = Utility_aToInt(src + 5 + 3 + 3 + 3);
	sec = Utility_aToInt(src + 5 + 3 + 3 + 3 + 3);
#else
	sscanf(src, "%04d-%02d-%02d%c%02d:%02d:%02d",
			&year, &month, &day, (char *)&sep, &hour, &min, &sec);
#endif
	// TODO: Something here??
	if (!(sep == ' ' || sep == 'T')) {
		// Date and Time is invalid (separator incorrect)
		return 0;
	}
	return timefn_getunixmillis(year, month, day, hour, min, sec);
}


