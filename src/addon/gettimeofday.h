/*
 * gettimeofday.h
 *
 *  Created on: 2015��8��31��
 *      Author: Stur
 */

#ifndef SRC_ADDON_DEPS_GETTIMEOFDAY_H_
#define SRC_ADDON_DEPS_GETTIMEOFDAY_H_

#ifdef _WIN32

#include <time.h>
#include <windows.h> 

#ifndef __cplusplus

typedef unsigned char bool;
const bool false = 0;
const bool true = 1;

#endif

/* IN UNIX the use of the timezone struct is obsolete;
 I don't know why you use it. See http://linux.about.com/od/commands/l/blcmdl2_gettime.htm
 But if you want to use this structure to know about GMT(UTC) diffrence from your local time
 it will be next: tz_minuteswest is the real diffrence in minutes from GMT(UTC) and a tz_dsttime is a flag
 indicates whether daylight is now in use
 */
struct timezone2
{
	__int32 tz_minuteswest; /* minutes W of Greenwich */
	bool tz_dsttime; /* type of dst correction */
};

struct timeval2 {
	__int32 tv_sec; /* seconds */
	__int32 tv_usec; /* microseconds */
};

int gettimeofday(struct timeval2 *tv/*in*/, struct timezone2 *tz/*in*/);
#endif

#endif /* SRC_ADDON_DEPS_GETTIMEOFDAY_H_ */

