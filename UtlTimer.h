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

To contact the author: codeworker@free.fr

This implementation is strongly inspired of 'SWORD::Chronograph', written by
Eric NICOLAS.
*/

#ifndef _UtlTimer_h_
#define _UtlTimer_h_

namespace CodeWorker {
	/*! A Chronograph for measuring time spent in code. Each
	 *  instance of this class accumulates time spent by your
	 *  program between calls to start() and stop(). The time
	 *  can then be retrieved by getTimeInMillis() and
	 *  getTimeInSec().
	 *
	 *  Example:
	 *  \code
	 *    Chronograph c;
	 *    c.start();
	 *    for(int i=0; i<10000; ++i) { / * do stuff * / }
	 *    c.stop();
	 *    cout << "Spent = " << c.getTimeInSec() << " seconds" << endl;
	 *  \endcode
	 */

#ifdef WIN32
	typedef signed   __int64       int64;
	typedef unsigned __int64       word64;
#else
	typedef signed   long long int int64;
	typedef unsigned long long int word64;
#endif

	class UtlTimer {
		int64  start_;
		int64  elapsed_;
		int64  freqInMillis_;
		double freqInSec_;
		bool   active_;

	public:
		/*! Creates a Chronograph. The time measurement is not
		 *  started yet. You must call start() once to start
		 *  the chronograph.
		 */
		inline UtlTimer() {
			clear();
			freqInMillis_ = freq() / 1000;
			freqInSec_    = (double)freq();
		}

		inline ~UtlTimer() {}

		/*! Clear (reset) the Chronograph. The accumulated elapsed
		 *  time is lost, and you have to call start() again to 
		 *  start measuring again
		 */
		inline void clear() {
			start_ = elapsed_ = 0;
			active_ = false;
		}

		/*! Start measuring time.
		 *  \see start, getTimeInMillis, getTimeInSec
		 */
		inline void start() {
			start_ = now();
			active_ = true;
		}

		/*! Stops measuring time. The time elapsed between the
		 *  last call to start() and this call is accumulated in
		 *  the chronograph
		 *  \see start, getTimeInMillis, getTimeInSec
		 */
		inline void stop() {
			elapsed_ += (now() - start_);
			active_ = false;
		}

		/*! Get the accumulated time in milliseconds. This method can
		 *  be called either 'during measurement' (between a start()
		 *  and a stop() ), or 'post morten' (after a stop() ).
		 *  \see getTimeInSec, start, stop
		 */
		unsigned long getTimeInMillis();

		/*! Get the accumulated time in seconds, with more precision
		 *  in the decimal part of the returned value. This method can
		 *  be called either 'during measurement' (between a start()
		 *  and a stop() ), or 'post morten' (after a stop() ).
		 *  \see getTimeInMillis, start, stop
		 */
		inline double getTimeInSec() {
			if (active_) {
				stop();
				start();
			}
			return (double) elapsed_ / freqInSec_;
		}

	private:
		// The two platform-dependant methods
		static int64 now();   // gets the current timestamp
		static int64 freq();  // gets the resolution of now()
	};
}

#endif
