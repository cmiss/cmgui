/**
 * FILE : cmgui_time.h
 *
 * Implements gettimeofday() and times() for non-Unix systems esp. WIN32_SYSTEM
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (GENERAL_CMGUI_TIME_HPP)
#define GENERAL_CMGUI_TIME_HPP

#include "configure/cmgui_configure.h"

#if defined (UNIX) /* switch (OPERATING_SYSTEM) */
#include <sys/time.h>
#include <sys/times.h>
#define cmgui_gettimeofday gettimeofday
#define cmgui_times times
#elif defined (WIN32_SYSTEM) /* switch (OPERATING_SYSTEM) */
#if defined (_MSC_VER)
	#ifndef _CRTDBG_MAP_ALLOC
		#define _CRTDBG_MAP_ALLOC
	#endif
	#include <stdlib.h>
	#include <crtdbg.h>
#endif /* defined (_MSC_VER) */
//#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int cmgui_gettimeofday_win32(struct timeval *time, void *timezone);

#ifdef __cplusplus
}
#endif /* __cplusplus */

typedef long clock_t;
struct tms 
{
	/* The times function in cmgui is just used to get a timestamp at
		the moment, if more than this is required it will need to be 
		implemented in the c function as well as added to this structure */
	int dummy;
};
clock_t cmgui_times_win32(struct tms *buffer);
#define cmgui_gettimeofday cmgui_gettimeofday_win32
#define cmgui_times cmgui_times_win32
#else /* switch (OPERATING_SYSTEM) */
#error "Need implementation of gettimeofday() and times() for this OS"
#endif /* switch (OPERATING_SYSTEM) */

#endif /* !defined (GENERAL_CMGUI_TIME_HPP) */
