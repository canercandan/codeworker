/*description:
{
	Header of the \textit{profiling module}.

	It must be expanded by the \textit{template-based} script called \samp{"RawProfilingHpp.cwt"}.
}
*/

#ifndef _RawProfiling_RawProfiling_h_
#define _RawProfiling_RawProfiling_h_

#include <string>

namespace RawProfiling {

	/*
		The following implementation is strongly inspired of 'SWORD::Chronograph',
		written by Eric NICOLAS.
	*/

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

	class Chronograph {
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
		inline Chronograph() {
			clear();
			freqInMillis_ = freq() / 1000;
			freqInSec_    = (double)freq();
		}

		inline ~Chronograph() {}

		/*! Clear (reset) the Chronograph. The accumulated elapsed
		 *  time is lost, and you have to call start() again to 
		 *  start measuring again
		 */
		inline void clear() {
			start_ = elapsed_ = 0;
			active_ = false;
		}

		/*! 'true' if the chronograph is measuring time
		 */
		inline bool active() const {
			return active_;
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


	class RawProfiling {
		private:
			Chronograph& theChronograph_;
			// to handle recursive calls: nothing to do with the
			// chronograph if it is already measuring time.
			bool recursivity_;

		public:
			inline RawProfiling(Chronograph& theChronograph) : theChronograph_(theChronograph) {
				recursivity_ = theChronograph_.active();
				if (!recursivity_) theChronograph_.start();
			}

			inline ~RawProfiling() {
				if (!recursivity_) theChronograph_.stop();
			}

			static void displayResults();
			static bool writeToHTML(const std::string& sFilename);

		public:
//##markup##"header declarations"
	};
}

#endif
