/*
 * win32.h
 *
 *  Created on: 2015年8月31日
 *      Author: Stur
 */

#ifndef SRC_ADDON_WIN32_H_
#define SRC_ADDON_WIN32_H_

#ifdef _WIN32
#include <windows.h>
#include "gettimeofday.h"
#define snprintf rpl_snprintf
typedef long long  c_long;
#else
typedef long c_long;
#endif


#endif /* SRC_ADDON_WIN32_H_ */
