#include <iostream>
#include <fstream>

#ifdef WIN32
#	include "windows.h"
#	include "winbase.h"
#else
#	include <sys/timeb.h>
#endif

#include "RawProfiling.h"


namespace RawProfiling {
	unsigned long Chronograph::getTimeInMillis() {
		if (active_) {
			stop();
			start();
		}			
		return (unsigned long) (elapsed_ / freqInMillis_);
	}

	int64 Chronograph::now() {
#ifdef WIN32
		LARGE_INTEGER value;
		QueryPerformanceCounter(&value);
		return (int64)value.QuadPart;
#else
		struct timeb tb;
		ftime(&tb);
		return (int64)tb.time * 1000 + (int64)tb.millitm;
#endif
	}

	int64 Chronograph::freq() {
#ifdef WIN32
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return (int64)freq.QuadPart;
#else
		return 1000;
#endif
	}


	void RawProfiling::displayResults() {
		std::cout << "TowersOfHanoi::start (" << TowersOfHanoi_start_counter << ", " << TowersOfHanoi_start_chronograph.getTimeInMillis() << "ms)" << std::endl;
		std::cout << "TowersOfHanoi::playNDisks (" << TowersOfHanoi_playNDisks_counter << ", " << TowersOfHanoi_playNDisks_chronograph.getTimeInMillis() << "ms)" << std::endl;
		std::cout << "TowersOfHanoi::moveDisk (" << TowersOfHanoi_moveDisk_counter << ", " << TowersOfHanoi_moveDisk_chronograph.getTimeInMillis() << "ms)" << std::endl;
		std::cout << "TowersOfHanoi::displayGame (" << TowersOfHanoi_displayGame_counter << ", " << TowersOfHanoi_displayGame_chronograph.getTimeInMillis() << "ms)" << std::endl;
	}

	bool RawProfiling::writeToHTML(const std::string& sFilename) {
		std::ofstream HTMLFile(sFilename.c_str(), std::ios::out | std::ios::binary);
		if (HTMLFile.fail()) return false;
		HTMLFile << "<html>" << std::endl;
		HTMLFile << "\t<body>" << std::endl;
		HTMLFile << "\t\t<h2>Profiling results</h2>" << std::endl;
		HTMLFile << "\t\t<table border=\"1\">" << std::endl;
		HTMLFile << "\t\t\t<tr>" << std::endl;
		HTMLFile << "\t\t\t\t<td><b>Function</b></td>" << std::endl;
		HTMLFile << "\t\t\t\t<td><b>Occurrences</b></td>" << std::endl;
		HTMLFile << "\t\t\t\t<td><b>Time in ms</b></td>" << std::endl;
		HTMLFile << "\t\t\t</tr>" << std::endl;
		HTMLFile << "\t\t\t<tr>" << std::endl;
		HTMLFile << "\t\t\t\t<td><i>TowersOfHanoi::start</i></td>" << std::endl;
		HTMLFile << "\t\t\t\t<td>" << TowersOfHanoi_start_counter << "</td>" << std::endl;
		HTMLFile << "\t\t\t\t<td>" << TowersOfHanoi_start_chronograph.getTimeInMillis() << "</td>" << std::endl;
		HTMLFile << "\t\t\t</tr>" << std::endl;
		HTMLFile << "\t\t\t<tr>" << std::endl;
		HTMLFile << "\t\t\t\t<td><i>TowersOfHanoi::playNDisks</i></td>" << std::endl;
		HTMLFile << "\t\t\t\t<td>" << TowersOfHanoi_playNDisks_counter << "</td>" << std::endl;
		HTMLFile << "\t\t\t\t<td>" << TowersOfHanoi_playNDisks_chronograph.getTimeInMillis() << "</td>" << std::endl;
		HTMLFile << "\t\t\t</tr>" << std::endl;
		HTMLFile << "\t\t\t<tr>" << std::endl;
		HTMLFile << "\t\t\t\t<td><i>TowersOfHanoi::moveDisk</i></td>" << std::endl;
		HTMLFile << "\t\t\t\t<td>" << TowersOfHanoi_moveDisk_counter << "</td>" << std::endl;
		HTMLFile << "\t\t\t\t<td>" << TowersOfHanoi_moveDisk_chronograph.getTimeInMillis() << "</td>" << std::endl;
		HTMLFile << "\t\t\t</tr>" << std::endl;
		HTMLFile << "\t\t\t<tr>" << std::endl;
		HTMLFile << "\t\t\t\t<td><i>TowersOfHanoi::displayGame</i></td>" << std::endl;
		HTMLFile << "\t\t\t\t<td>" << TowersOfHanoi_displayGame_counter << "</td>" << std::endl;
		HTMLFile << "\t\t\t\t<td>" << TowersOfHanoi_displayGame_chronograph.getTimeInMillis() << "</td>" << std::endl;
		HTMLFile << "\t\t\t</tr>" << std::endl;
		HTMLFile << "\t\t</table>" << std::endl;
		HTMLFile << "\t</body>" << std::endl;
		HTMLFile << "</html>" << std::endl;
		HTMLFile.close();
		return true;
	}


	int RawProfiling::TowersOfHanoi_start_counter = 0;
	Chronograph RawProfiling::TowersOfHanoi_start_chronograph = Chronograph();
	int RawProfiling::TowersOfHanoi_playNDisks_counter = 0;
	Chronograph RawProfiling::TowersOfHanoi_playNDisks_chronograph = Chronograph();
	int RawProfiling::TowersOfHanoi_moveDisk_counter = 0;
	Chronograph RawProfiling::TowersOfHanoi_moveDisk_chronograph = Chronograph();
	int RawProfiling::TowersOfHanoi_displayGame_counter = 0;
	Chronograph RawProfiling::TowersOfHanoi_displayGame_chronograph = Chronograph();
}
