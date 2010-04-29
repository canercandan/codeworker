
/* "CodeWorker":	a scripting language for parsing and generating text.

Copyright (C) 1996-1997, 1999-2002 Cédric Lemaire

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

This implementation is inspired of 'ABE::Chronograph', written by Eric NICOLAS
*/// Various os-dependant implementations of Chronograph
// WIN32 :
//   QueryPerformanceCounter
//   QueryPerformanceFrequency
// UNIX :
//   gettimeofday

#ifdef WIN32
#	include <windows.h>
#	include <winbase.h>
#else
#	include <time.h>
#	include <sys/time.h>
#endif

#include "UtlTimer.h"

namespace CodeWorker {
	unsigned long UtlTimer::getTimeInMillis() {
		if (active_) {
			stop();
			start();
		}
		return (unsigned long) (elapsed_ / freqInMillis_);
	}

	int64 UtlTimer::now() {
#ifdef WIN32
		LARGE_INTEGER value;
		QueryPerformanceCounter(&value);
		return (int64)value.QuadPart;
#else
        struct timeval tv;
		gettimeofday(&tv, NULL);
		return (int64)tv.tv_sec * 1000000 + (int64)tv.tv_usec;
#endif
	}

	int64 UtlTimer::freq() {
#ifdef WIN32
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return (int64)freq.QuadPart;
#else
		return (int64)1000000;
#endif
	}
}
