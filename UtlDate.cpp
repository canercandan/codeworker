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
*/

#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef WIN32
#	include <windows.h>
#endif

#include "UtlException.h"
#include "UtlDate.h"
#include "UtlString.h"

const double WINGZ_INFINITY = 1.0e300;
const int INFINITE_YEAR = 2000000000;

namespace CodeWorker {
	class UtlInternalDate {
	public:
		static const char* _tsWeekDays[7];
		static const char* _tsMonths[12];
		static int _tNbDays[12];

	private:
		int _iDay;
		int _iMonth;
		int _iYear;
		int _iHour;
		int _iMin;
		int _iSec;
		int _iMillis;
		std::string _sComment;

	private:
		static std::string formatInteger(int iNumber, unsigned int iDigits) {
			char sNumber[32];
			sprintf(sNumber, "%d", iNumber);
			std::string sResult = sNumber;
			while (sResult.size() < iDigits) sResult = "0" + sResult;
			return sResult;
		}

	public:
		UtlInternalDate() {
#ifdef WIN32
			SYSTEMTIME mySystemTime;
			GetLocalTime(&mySystemTime);
			_iDay = mySystemTime.wDay;
			_iMonth = mySystemTime.wMonth;
			_iYear = mySystemTime.wYear;
			_iHour = mySystemTime.wHour;
			_iMin = mySystemTime.wMinute;
			_iSec = mySystemTime.wSecond;
			_iMillis = mySystemTime.wMilliseconds;
#else
			time_t timeInSecSince1970;
			time(&timeInSecSince1970);
			struct tm* today;
			struct tm xTIME;
			today = localtime_r(&timeInSecSince1970, &xTIME);
			_iDay = today->tm_mday;
			_iMonth = today->tm_mon + 1;
			_iYear = today->tm_year + 1900;
			_iHour = today->tm_hour;
			_iMin = today->tm_min;
			_iSec = today->tm_sec;
			_iMillis = 0;
#endif
		}
		UtlInternalDate(double dWINGZDate) {
			setWINGZDate(dWINGZDate);
		}
		UtlInternalDate(int iWinfrontDate) {
			setWinfrontDate(iWinfrontDate);
		}
		UtlInternalDate(int iDay, int iMonth, int iYear, int iHour, int iMin, int iSec, int iMillis, const std::string& sComment) : _iDay(iDay), _iMonth(iMonth), _iYear(iYear), _iHour(iHour), _iMin(iMin), _iSec(iSec), _iMillis(iMillis), _sComment(sComment) {}
		virtual ~UtlInternalDate() {}

		virtual UtlInternalDate* copyInstance() const { return new UtlInternalDate(_iDay, _iMonth, _iYear, _iHour, _iMin, _iSec, _iMillis, _sComment); }

		bool isInfinite() const {
			return (getYear() >= INFINITE_YEAR);
		}

		const std::string& getComment() const { return _sComment; }
		void setComment(const std::string& sComment) { _sComment = sComment; }

		virtual bool isAFixedDate() const { return true; }
		virtual bool isAVariableDate() const { return false; }

		virtual int getWeekDay() const { return (getDayWINGZDate() + 6) % 7; }
		virtual int getDay() const { return _iDay; }
		virtual int getMonth() const { return _iMonth; }
		virtual int getYear() const { return _iYear; }

		virtual void setDay(int iDay) { _iDay = iDay; }
		virtual void setMonth(int iMonth) { _iMonth = iMonth; }
		virtual void setYear(int iYear) { _iYear = iYear; }

		void addDay(int iDay) {
			_iDay += iDay;
			if (_iDay > 0) {
				do {
					int iNbDays = getNbDaysInMonth(_iMonth, _iYear);
					if (_iDay > iNbDays) {
						_iDay -= iNbDays;
						_iMonth++;
						if (_iMonth > 12) {
							_iMonth = 1;
							_iYear++;
						}
					} else break;
				} while (true);
			} else {
				while (_iDay < 1) {
					_iMonth--;
					if (_iMonth <= 0) {
						_iMonth = 12;
						_iYear--;
					}
					int iNbDays = getNbDaysInMonth(_iMonth, _iYear);
					_iDay += iNbDays;
				}
			}
		}

		void addMonth(int iMonth) {
			_iMonth += iMonth;
			if (_iMonth <= 0) {
				_iYear += ((_iMonth - 1) / 12) - 1;
				_iMonth = (12 - ((1-_iMonth)%12))%12;
			} else { 
				_iYear += ((_iMonth - 1) / 12);
				_iMonth = (_iMonth - 1) % 12;
			}
			_iMonth++;
			int iDaysOfMonth = getNbDaysInMonth(_iMonth, _iYear);
			if (iDaysOfMonth < _iDay) _iDay = iDaysOfMonth;
		}

		void addYear(int iYear) {
			_iYear += iYear;
			int iDaysOfMonth = getNbDaysInMonth(_iMonth, _iYear);
			if (iDaysOfMonth < _iDay) _iDay = iDaysOfMonth;
		}

		void setTime(int iHour, int iMin, int iSec, int iMillis) {
			_iHour = iHour;
			_iMin = iMin;
			_iSec = iSec;
			_iMillis = iMillis;
		}

		inline int getHour() const { return _iHour; }
		inline void setHour(int iHour) { _iHour = iHour; }
		inline int getMin() const { return _iMin; }
		inline void setMin(int iMin) { _iMin = iMin; }
		inline int getSec() const { return _iSec; }
		inline void setSec(int iSec) { _iSec = iSec; }
		inline int getMillis() const { return _iMillis; }
		inline void setMillis(int iMillis) { _iMillis = iMillis; }
		virtual bool instanceOf(const char *pName) {
			if (!strcmp(pName, "UtlInternalDate") ) return true ;
				else return false;
		}

		void addHour(int iHour) {
			_iHour += iHour;
			int iNbDays = _iHour / 24;
			_iHour = _iHour % 24;
			if (_iHour < 0) {
				_iHour += 24;
				iNbDays--;
			}
			addDay(iNbDays);
		}

		void addMin(int iMin) {
			_iMin += iMin;
			int iNbHours = _iMin / 60;
			_iMin = _iMin % 60;
			if (_iMin < 0) {
				_iMin += 60;
				iNbHours--;
			}
			addHour(iNbHours);
		}

		void addSec(int iSec) {
			_iSec += iSec;
			int iNbMins = _iSec / 60;
			_iSec = _iSec % 60;
			if (_iSec < 0) {
				_iSec += 60;
				iNbMins--;
			}
			addMin(iNbMins);
		}

		void addMillis(int iMillis) {
			_iMillis += iMillis;
			int iNbSecs = _iMillis / 1000;
			_iMillis = _iMillis % 1000;
			if (_iMillis < 0) {
				_iMillis += 1000;
				iNbSecs--;
			}
			addSec(iNbSecs);
		}

		virtual bool isLastDayOfMonth() const {
			int iDaysOfMonth = getNbDaysInMonth(_iMonth, _iYear);
			return (getDay() == iDaysOfMonth);
		}

		virtual void setLastDayOfMonth() {
			int iDaysOfMonth = getNbDaysInMonth(_iMonth, _iYear);
			setDay(iDaysOfMonth);
		}

		virtual std::string getString() const {
			char tcString[32];
			std::string sMonth = _tsMonths[_iMonth - 1];
			sMonth = sMonth.substr(0, 3);
			if ((_iHour > 0) || (_iMin > 0) || (_iSec > 0) || (_iMillis > 0)) {
				sprintf(tcString, "%02d%s%d %02d:%02d:%02d.%03d", _iDay, sMonth.c_str(), _iYear, _iHour, _iMin, _iSec, _iMillis);
			} else {
				sprintf(tcString, "%02d%s%d", _iDay, sMonth.c_str(), _iYear);
			}
			return tcString;
		}

		std::string getMCLDate() const {
			char sMCLString[16];
			std::string sMonth = _tsMonths[_iMonth - 1];
			sMonth = sMonth.substr(0, 3);
			sprintf(sMCLString, "%d%s%d", _iDay, sMonth.c_str(), _iYear);
			return sMCLString;
		}

		std::string getFormattedDate(const std::string& sFormat) const {
			std::string sDate;
			std::string::size_type i = 0;
			while (i < sFormat.size()) {
				if (sFormat[i] == '%') {
					i++;
					switch(sFormat[i]) {
						case 'Y': sDate += formatInteger(_iYear, 4);break;
						case 'y': sDate += formatInteger(_iYear % 100, 2);break;
						case 'b': sDate += std::string(_tsMonths[_iMonth - 1]).substr(0, 3);break;
						case 'B': sDate += _tsMonths[_iMonth - 1];break;
						case 'm': sDate += formatInteger(_iMonth, 2);break;
						case 'd': sDate += formatInteger(_iDay, 2);break;
						case 'e': sDate += formatInteger(_iDay, 1);break;
						case 't': {
									char sNumber[300];
									double dWingz = getWINGZDate();
									if (dWingz == floor(dWingz)) sprintf(sNumber, "%d", (int) dWingz);
									else sprintf(sNumber, "%f", dWingz);
									sDate += sNumber;
									break;
								  }
						case 'w': sDate += formatInteger(getWeekDay(), 1);break;
						case 'W': sDate += std::string(_tsWeekDays[getWeekDay()]);break;
						case 'H': sDate += formatInteger(_iHour, 2);break;
						case 'I': sDate += formatInteger(((_iHour > 12) ? _iHour - 12: _iHour), 2);break;
						case 'p': sDate += ((_iHour > 12) ? "PM": "AM");break;
						case 'Q':
								sDate += formatInteger(_iYear, 4) + "-" + formatInteger(_iMonth, 2) + "-" + formatInteger(_iDay, 2) + " " + formatInteger(_iHour, 2) + ":" + formatInteger(_iMin, 2) + ":" + formatInteger(_iSec, 2);
								if (_iMillis != 0) sDate += "." + formatInteger(_iMillis,  3);
								break;
						case 'M': sDate += formatInteger(_iMin,  2);break;
						case 'S': sDate += formatInteger(_iSec,  2);break;
						case 'L': sDate += formatInteger(_iMillis,  3);break;
						case '%': sDate += "%";break;
						case 'j':
							{
								int iDays = _iDay;
								int iMonth = 1;
								while (iMonth < _iMonth) iDays += getNbDaysInMonth(iMonth++, _iYear);
								sDate += formatInteger(iDays, 1);
							}
							break;
						case 'D': sDate += formatInteger(_iMonth, 2) + "/" + formatInteger(_iDay, 2) + "/" + formatInteger(_iYear % 100, 2);break;
						case 'r': sDate += formatInteger(((_iHour > 12) ? _iHour - 12: _iHour), 2) + ":" + formatInteger(_iMin, 2) + ":" + formatInteger(_iSec, 2) + ((_iHour > 12) ? " PM": " AM");break;
						case 'T': sDate += formatInteger(_iHour, 2) + ":" + formatInteger(_iMin, 2) + ":" + formatInteger(_iSec, 2);break;
						default:
							throw UtlException("syntax error on date format \"" + sFormat + "\"");
					}
				} else sDate += sFormat[i];
				i++;
			}
			return sDate;
		}

		int getDayWINGZDate() const { return (int) getWINGZDate(); }

		double getWINGZDate() const {
			if (isInfinite()) return WINGZ_INFINITY;
			int iWingzDate = 2;
			int iDateYear = getYear();
			if (iDateYear < 1900) return -1.0;

			int iYear = 1900;
			while (iYear < iDateYear) {
				bool b366 = isLeapYear(iYear);
				if (b366) iWingzDate += 366;
				else iWingzDate += 365;
				iYear++;
			}

	  		int iMonth = 1;
			int iDateMonth = getMonth();
	  		while (iMonth < iDateMonth) {
				int iNbDays = getNbDaysInMonth(iMonth, iYear);
				iWingzDate += iNbDays;
				iMonth++;
	  		}
	  		
	  		iWingzDate += (getDay() - 1);
			double dMillis = _iMillis;
			double dSeconds = (dMillis / 1000.0) + _iSec + 60 * (_iMin + 60*_iHour);
			double dWingzDate = iWingzDate;
			dWingzDate += dSeconds / 86400.0;

			return dWingzDate;
		}

		void setWINGZDate(double dWINGZDate) {
			if (dWINGZDate == WINGZ_INFINITY) {
				_iYear = INFINITE_YEAR;
			} else {
				_iDay = 1;
				_iMonth = 1;
				_iYear =1900;
				addDay((int) (dWINGZDate - 2.0));
				double dRemaining = dWINGZDate - floor(dWINGZDate);
				dRemaining *= 24.0;
				double dFloor = floor(dRemaining);
				_iHour = (int) dFloor;
				dRemaining -= dFloor;
				dRemaining *= 60.0;
				dFloor = floor(dRemaining);
				_iMin = (int) dFloor;
				dRemaining -= dFloor;
				dRemaining *= 60.0;
				dFloor = floor(dRemaining);
				_iSec = (int) dFloor;
				dRemaining -= dFloor;
				dRemaining *= 1000.0;
				double dMillis = dRemaining;
				_iMillis = (int) floor(dMillis);
				if (dMillis - _iMillis >= 0.5) {
					_iMillis++;
					if (_iMillis == 1000) {
						_iMillis = 0;
						_iSec++;
						if (_iSec == 60) {
							_iSec = 0;
							_iMin++;
							if (_iMin == 60) {
								_iMin = 0;
								_iHour++;
								if (_iHour == 24) {
									_iHour = 0;
									_iDay++;
									if (getNbDaysInMonth(_iMonth, _iYear) < _iDay) {
										_iDay = 1;
										_iMonth++;
										if (_iMonth > 12) {
											_iMonth = 1;
											_iYear++;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		int getWinfrontDate() const {
			return getYear() * 10000 + getMonth() * 100 + getDay();
		}

		void setWinfrontDate(int iWinfrontDate) {
			setDay(iWinfrontDate % 100);
			iWinfrontDate /= 100;
			setMonth(iWinfrontDate % 100);
			iWinfrontDate /= 100;
			setYear(iWinfrontDate);
			_iHour   = 0;
			_iMin    = 0;
			_iSec    = 0;
			_iMillis = 0;
		}

		static int getNbDaysInMonth(int iMonth, int iYear) {
			int iNbDays = _tNbDays[iMonth - 1];
			if ((iMonth == 2) && isLeapYear(iYear)) iNbDays = 29;
			return iNbDays;
		}

		static bool isLeapYear(int iYear) { return (iYear%4 == 0) && ((iYear%100 != 0) || (iYear%400 == 0)); }
	};


	const char* UtlInternalDate::_tsWeekDays[7] = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
	const char* UtlInternalDate::_tsMonths[12] = {"january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december"};
	int UtlInternalDate::_tNbDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	class UtlVariableDate : public UtlInternalDate {
	private:
		int  _iFrequency;
		char _cUnit;

	public:
		UtlVariableDate(int iFrequency, char cUnit) : _iFrequency(iFrequency), _cUnit(cUnit) {
			char sErrorMessage[128];
			switch(_cUnit) {
				case 'd':
				case 'w':
				case 'm':
				case 'y':
					_cUnit -= ' ';
				case 'D':
				case 'W':
				case 'M':
				case 'Y':
					break;
				default:
					sprintf(sErrorMessage, "Undefined unit of variable date: '%c' for \"%d%c\"", cUnit, iFrequency, cUnit);
					throw UtlException(sErrorMessage);
			}
		}
		virtual ~UtlVariableDate() {}

		virtual UtlInternalDate* copyInstance() const { return new UtlVariableDate(_iFrequency, _cUnit); }

		virtual bool isAFixedDate() const { return false; }
		virtual bool isAVariableDate() const { return true; }

		virtual int getWeekDay() const {
			UtlInternalDate date(UtlInternalDate::getDay(), UtlInternalDate::getMonth(), UtlInternalDate::getYear(), 0, 0, 0, 0, "");
			switch (_cUnit) {
				case 'D': date.addDay(_iFrequency);break;
				case 'W': date.addDay(7*_iFrequency);break;
				case 'M': date.addMonth(_iFrequency);break;
				case 'Y': date.addYear(_iFrequency);break;
			}
			return date.getWeekDay();
		}

		virtual int getDay() const {
			UtlInternalDate date(UtlInternalDate::getDay(), UtlInternalDate::getMonth(), UtlInternalDate::getYear(), 0, 0, 0, 0, "");
			switch (_cUnit) {
				case 'D': date.addDay(_iFrequency);break;
				case 'W': date.addDay(7*_iFrequency);break;
				case 'M': date.addMonth(_iFrequency);break;
				case 'Y': date.addYear(_iFrequency);break;
			}
			return date.getDay();
		}

		virtual int getMonth() const {
			UtlInternalDate date(UtlInternalDate::getDay(), UtlInternalDate::getMonth(), UtlInternalDate::getYear(), 0, 0, 0, 0, "");
			switch (_cUnit) {
				case 'D': date.addDay(_iFrequency);break;
				case 'W': date.addDay(7*_iFrequency);break;
				case 'M': date.addMonth(_iFrequency);break;
				case 'Y': date.addYear(_iFrequency);break;
			}
			return date.getMonth();
		}

		virtual int getYear() const {
			UtlInternalDate date(UtlInternalDate::getDay(), UtlInternalDate::getMonth(), UtlInternalDate::getYear(), 0, 0, 0, 0, "");
			switch (_cUnit) {
				case 'D': date.addDay(_iFrequency);break;
				case 'W': date.addDay(7*_iFrequency);break;
				case 'M': date.addMonth(_iFrequency);break;
				case 'Y': date.addYear(_iFrequency);break;
			}
			return date.getYear();
		}

		virtual std::string getString() const {
			char sString[16];
			sprintf(sString, "%d%c", _iFrequency, _cUnit);
			return sString;
		}

		virtual int getFrequency() const {
			return _iFrequency	;
		}
		
		virtual char getUnit() const {
			return _cUnit	;
		}

		virtual bool instanceOf(const char *pName) {
			if (!strcmp(pName, "UtlVariableDate") ) return true ;
				else return false;
		}
	};


	UtlDate::UtlDate() {
		_pInternalDate = new UtlInternalDate();
	}

	UtlDate::UtlDate(const UtlDate& date) {
		if (date._pInternalDate == NULL) _pInternalDate = NULL;
		else _pInternalDate = date._pInternalDate->copyInstance();
	}

	UtlDate::UtlDate(double dWINGZDate) {
		if (dWINGZDate <= 0.0) _pInternalDate = NULL;
		else _pInternalDate = new UtlInternalDate(dWINGZDate);
	}

	UtlDate::UtlDate(int iWinfrontDate) {
		_pInternalDate = new UtlInternalDate(iWinfrontDate);
	}

	UtlDate::UtlDate(int iDay, int iMonth, int iYear, int iHour, int iMin, int iSec, int iMillis) {
		_pInternalDate = new UtlInternalDate(iDay, iMonth, iYear, iHour, iMin, iSec, iMillis, "");
	}

	UtlDate::UtlDate(int iFrequency, char cUnit) {
		_pInternalDate = new UtlVariableDate(iFrequency, cUnit);
	}

	UtlDate::~UtlDate() {
		delete _pInternalDate;
	}

	bool UtlDate::isNull() const {
		return (_pInternalDate == NULL);
	}

	void UtlDate::nullify() {
		if (_pInternalDate != NULL) delete _pInternalDate;
		_pInternalDate = NULL;
	}

	bool UtlDate::isInfinite() const {
		if (_pInternalDate == NULL) return false;
		return _pInternalDate->isInfinite();
	}

	void UtlDate::infinity() {
		if (_pInternalDate != NULL) delete _pInternalDate;
		_pInternalDate = new UtlInternalDate(WINGZ_INFINITY);
	}

	UtlDate* UtlDate::copyInstance() const {
		return new UtlDate(*this);
	}

	UtlDate& UtlDate::operator =(const UtlDate& date) {
		if (&date == this) throw UtlException("a date can't assign itself");
		if (_pInternalDate != NULL) delete _pInternalDate;
		if (date._pInternalDate == NULL) _pInternalDate = NULL;
		else _pInternalDate = date._pInternalDate->copyInstance();
		return *this;
	}

	bool UtlDate::operator <(const UtlDate& date) const {
		if (isNull() || date.isNull()) return false;
		int iYear1 = getYear();
		int iYear2 = date.getYear();
		if (iYear1 == iYear2) {
			int iMonth1 = getMonth();
			int iMonth2 = date.getMonth();
			if (iMonth1 != iMonth2) return (iMonth1 < iMonth2);
			int iDay1 = getDay();
			int iDay2 = date.getDay();
			if (iDay1 != iDay2) return (iDay1 < iDay2);
			int iHour1 = getHour();
			int iHour2 = date.getHour();
			if (iHour1 != iHour2) return (iHour1 < iHour2);
			int iMin1 = getMin();
			int iMin2 = date.getMin();
			if (iMin1 != iMin2) return (iMin1 < iMin2);
			int iSec1 = getSec();
			int iSec2 = date.getSec();
			if (iSec1 != iSec2) return (iSec1 < iSec2);
		} else return (iYear1 < iYear2);
		return getMillis() < date.getMillis();
	}

	bool UtlDate::operator <=(const UtlDate& date) const {
		if (isNull() || date.isNull()) return false;
		int iYear1 = getYear();
		int iYear2 = date.getYear();
		if (iYear1 == iYear2) {
			int iMonth1 = getMonth();
			int iMonth2 = date.getMonth();
			if (iMonth1 != iMonth2) return (iMonth1 < iMonth2);
			int iDay1 = getDay();
			int iDay2 = date.getDay();
			if (iDay1 != iDay2) return (iDay1 < iDay2);
			int iHour1 = getHour();
			int iHour2 = date.getHour();
			if (iHour1 != iHour2) return (iHour1 < iHour2);
			int iMin1 = getMin();
			int iMin2 = date.getMin();
			if (iMin1 != iMin2) return (iMin1 < iMin2);
			int iSec1 = getSec();
			int iSec2 = date.getSec();
			if (iSec1 != iSec2) return (iSec1 < iSec2);
		} else return (iYear1 < iYear2);
		return getMillis() <= date.getMillis();
	}

	bool UtlDate::operator ==(const UtlDate& date) const {
		if (isNull() || date.isNull()) return false;
		return (getDay() == date.getDay()) && (getMonth() == date.getMonth()) && (getYear() == date.getYear()) && (getHour() == date.getHour()) && (getMin() == date.getMin()) && (getSec() == date.getSec()) && (getMillis() == date.getMillis());
	}

	bool UtlDate::operator !=(const UtlDate& date) const {
		if (isNull() || date.isNull()) return false;
		return (getDay() != date.getDay()) || (getMonth() != date.getMonth()) || (getYear() != date.getYear()) || (getHour() != date.getHour()) || (getMin() != date.getMin()) || (getSec() != date.getSec()) || (getMillis() != date.getMillis());
	}

	bool UtlDate::operator >=(const UtlDate& date) const {
		if (isNull() || date.isNull()) return false;
		int iYear1 = getYear();
		int iYear2 = date.getYear();
		if (iYear1 == iYear2) {
			int iMonth1 = getMonth();
			int iMonth2 = date.getMonth();
			if (iMonth1 != iMonth2) return (iMonth1 > iMonth2);
			int iDay1 = getDay();
			int iDay2 = date.getDay();
			if (iDay1 != iDay2) return (iDay1 > iDay2);
			int iHour1 = getHour();
			int iHour2 = date.getHour();
			if (iHour1 != iHour2) return (iHour1 > iHour2);
			int iMin1 = getMin();
			int iMin2 = date.getMin();
			if (iMin1 != iMin2) return (iMin1 > iMin2);
			int iSec1 = getSec();
			int iSec2 = date.getSec();
			if (iSec1 != iSec2) return (iSec1 > iSec2);
		} else return (iYear1 > iYear2);
		return getMillis() >= date.getMillis();
	}

	bool UtlDate::operator >(const UtlDate& date) const {
		if (isNull() || date.isNull()) return false;
		int iYear1 = getYear();
		int iYear2 = date.getYear();
		if (iYear1 == iYear2) {
			int iMonth1 = getMonth();
			int iMonth2 = date.getMonth();
			if (iMonth1 != iMonth2) return (iMonth1 > iMonth2);
			int iDay1 = getDay();
			int iDay2 = date.getDay();
			if (iDay1 != iDay2) return (iDay1 > iDay2);
			int iHour1 = getHour();
			int iHour2 = date.getHour();
			if (iHour1 != iHour2) return (iHour1 > iHour2);
			int iMin1 = getMin();
			int iMin2 = date.getMin();
			if (iMin1 != iMin2) return (iMin1 > iMin2);
			int iSec1 = getSec();
			int iSec2 = date.getSec();
			if (iSec1 != iSec2) return (iSec1 > iSec2);
		} else return (iYear1 > iYear2);
		return getMillis() > date.getMillis();
	}

	double UtlDate::operator -(const UtlDate& date) const {
		if (isNull() || date.isNull()) return 0.0;
		return getWINGZDate() - date.getWINGZDate();
	}

	void UtlDate::today() {
		if (_pInternalDate != NULL) delete _pInternalDate;
		_pInternalDate = new UtlInternalDate();
	}

	bool UtlDate::isAFixedDate() const {
		if (isNull()) return false;
		return _pInternalDate->isAFixedDate();
	}
	bool UtlDate::isAVariableDate() const {
		if (isNull()) return false;
		return _pInternalDate->isAVariableDate();
	}

	int UtlDate::getDay() const {
		return _pInternalDate->getDay();
	}

	int UtlDate::getMonth() const {
		return _pInternalDate->getMonth();
	}

	int UtlDate::getYear() const {
		return _pInternalDate->getYear();
	}

	void UtlDate::setDay(int iDay) {
		_pInternalDate->setDay(iDay);
	}

	void UtlDate::setMonth(int iMonth) {
		_pInternalDate->setMonth(iMonth);
	}

	void UtlDate::setYear(int iYear) {
		_pInternalDate->setYear(iYear);
	}

	void UtlDate::addDay(int iDay) {
		_pInternalDate->addDay(iDay);
	}

	void UtlDate::addMonth(int iMonth) {
		_pInternalDate->addMonth(iMonth);
	}

	void UtlDate::addYear(int iYear) {
		_pInternalDate->addYear(iYear);
	}

	int UtlDate::getHour() const {
		return _pInternalDate->getHour();
	}

	int UtlDate::getMin() const {
		return _pInternalDate->getMin();
	}

	int UtlDate::getSec() const {
		return _pInternalDate->getSec();
	}

	int UtlDate::getMillis() const {
		return _pInternalDate->getMillis();
	}

	bool UtlDate::isLastDayOfMonth() const {
		return _pInternalDate->isLastDayOfMonth();
	}

	void UtlDate::setLastDayOfMonth() {
		_pInternalDate->setLastDayOfMonth();
	}

	void UtlDate::ignoreHours() {
		_pInternalDate->setHour(0);
		_pInternalDate->setMin(0);
		_pInternalDate->setSec(0);
		_pInternalDate->setMillis(0);
	}

	void UtlDate::setTime(int iHour, int iMin, int iSec, int iMillis) {
		_pInternalDate->setTime(iHour, iMin, iSec, iMillis);
	}

	int UtlDate::compareTime(const UtlDate& date) const {
		int iHour1 = getHour();
		int iHour2 = date.getHour();
		if (iHour1 != iHour2) return (iHour1 - iHour2);
		int iMin1 = getMin();
		int iMin2 = date.getMin();
		if (iMin1 != iMin2) return (iMin1 - iMin2);
		int iSec1 = getSec();
		int iSec2 = date.getSec();
		if (iSec1 != iSec2) return (iSec1 - iSec2);
		return getMillis() - date.getMillis();
	}

	void UtlDate::setHour(int iHour) {
		_pInternalDate->setHour(iHour);
	}

	void UtlDate::setMin(int iMin) {
		_pInternalDate->setMin(iMin);
	}

	void UtlDate::setSec(int iSec) {
		_pInternalDate->setSec(iSec);
	}

	void UtlDate::setMillis(int iMillis) {
		_pInternalDate->setMillis(iMillis);
	}

	void UtlDate::addHour(int iHour) {
		_pInternalDate->addHour(iHour);
	}

	void UtlDate::addMin(int iMin) {
		_pInternalDate->addMin(iMin);
	}

	void UtlDate::addSec(int iSec) {
		_pInternalDate->addSec(iSec);
	}

	void UtlDate::addMillis(int iMillis) {
		_pInternalDate->addMillis(iMillis);
	}

	std::string UtlDate::getString() const {
		if (isNull()) return "undated";
		return _pInternalDate->getString();
	}

	std::string UtlDate::getMCLDate() const {
		if (isNull()) return "undated";
		return _pInternalDate->getMCLDate();
	}

	std::string UtlDate::getFormattedDate(const std::string& sFormat) const {
		return _pInternalDate->getFormattedDate(sFormat);
	}

	UtlDate UtlDate::getDateFromFormat(const std::string& sDate, const std::string& sFormat) {
		UtlDate today;
		today.ignoreHours();
		std::string::size_type i = 0;
		std::string::size_type j = 0;
		std::string sNumber;
		while ((i < sFormat.size()) && (j < sDate.size())) {
			if (sFormat[i] == '%') {
				i++;
				switch(sFormat[i]) {
					case 'Y':
						if (j + 4 > sDate.size()) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Y'");
						sNumber = sDate.substr(j, 4);
						today.setYear(atoi(sNumber.c_str()));
						j += 4;
						break;
					case 'y':
						{
							int iYear = getInteger(sDate, 2, j);
							int iTodayShortYear = today.getYear() % 100;
							if (iTodayShortYear > iYear) {
								if (iTodayShortYear - iYear > 50) iYear += 100;
							} else {
								if (iYear - iTodayShortYear > 50) iYear -= 100;
							}
							iYear += (today.getYear() - iTodayShortYear);
							today.setYear(iYear);
						}
						break;
					case 'm':
						today.setMonth(getInteger(sDate, 2, j));
						if ((today.getMonth() < 1) || (today.getMonth() > 12)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%m'");
						break;
					case 'd':
						today.setDay(getInteger(sDate, 2, j));
						if ((today.getDay() < 1) || (today.getDay() > 31)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%d'");
						break;
					case 'H':
						today.setHour(getInteger(sDate, 2, j));
						if ((today.getHour() < 0) || (today.getHour() > 23)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%H'");
						break;
					case 'I':
						if (j + 2 > sDate.size()) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%I'");
						sNumber = sDate.substr(j, 2);
						today.setHour(atoi(sNumber.c_str()));
						j += 2;
						break;
					case 'M':
						today.setMin(getInteger(sDate, 2, j));
						if ((today.getMin() < 0) || (today.getMin() > 59)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%M'");
						break;
					case 'S':
						today.setSec(getInteger(sDate, 2, j));
						if ((today.getSec() < 0) || (today.getSec() > 59)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%S'");
						break;
					case '%': break;
					case 'D':
						today.setMonth(getInteger(sDate, 2, j));
						if ((today.getMonth() < 1) || (today.getMonth() > 12)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%D -> %m'");
						if (sDate[j++] != '/') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%D -> '/' expected after %m'");
						today.setDay(getInteger(sDate, 2, j));
						if ((today.getDay() < 1) || (today.getDay() > 31)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%D -> %d'");
						if (sDate[j++] != '/') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%D -> '/' expected after %d'");
						{
							int iYear = getInteger(sDate, 2, j);
							if (iYear > 70) iYear += 1900;
							else iYear += 2000;
							today.setYear(iYear);
						}
						break;
					case 'Q':
						today.setYear(getInteger(sDate, 4, j));
						if (sDate[j++] != '-') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> '-' expected after %Y'");
						today.setMonth(getInteger(sDate, 2, j));
						if ((today.getMonth() < 1) || (today.getMonth() > 12)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> %m'");
						if (sDate[j++] != '-') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> '-' expected after %m'");
						today.setDay(getInteger(sDate, 2, j));
						if ((today.getDay() < 1) || (today.getDay() > 31)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> %d'");
						if (sDate[j++] != ' ') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> ' ' expected after %d'");
						today.setHour(getInteger(sDate, 2, j));
						if ((today.getHour() < 0) || (today.getHour() > 23)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> %H'");
						if (sDate[j++] != ':') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> ':' expected after %H'");
						today.setMin(getInteger(sDate, 2, j));
						if ((today.getMin() < 0) || (today.getMin() > 59)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> %M'");
						if (sDate[j++] != ':') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> ':' expected after %M'");
						today.setSec(getInteger(sDate, 2, j));
						if ((today.getSec() < 0) || (today.getSec() > 59)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> %S'");
						if (sDate[j] != '.') break;
						j++;
						// Be careful!
						// Continue in sequence on case '%L'!
					case 'L':
						// Be careful!
						// Don't move this case label and don't insert anything before case '%Q'
						{
							if (j >= sDate.size()) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%L'");
							char a = sDate[j];
							if ((a < '0') || (a > '9')) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%L'");
							int iMillis = 0;
							int iLoop = 0;
							do {
								iMillis = 10*iMillis + (a - '0');
								a = sDate[++j];
							} while ((++iLoop < 3) && (a >= '0') && (a <= '9'));
							today.setMillis(iMillis);
						}
						break;
					case 'b':
						if (j + 3 > sDate.size()) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%b'");
						sNumber = sDate.substr(j, 3);
						{
							int k;
							for (k = 0; k < 12; k++) if (strnicmp(UtlInternalDate::_tsMonths[k], sNumber, 3) == 0) break;
							if (k >= 12) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%b' -> unknow month = '" + sNumber + "'");
							today.setMonth(k + 1);
						}
						j += 3;
						break;
					case 'B':
						{
							int k;
							int iLength;
							for (k = 0; k < 12; k++) {
								iLength = strlen(UtlInternalDate::_tsMonths[k]);
								if (j + iLength <= sDate.size()) {
									if (strnicmp(UtlInternalDate::_tsMonths[k], sDate.c_str() + j, iLength) == 0) break;
								}
							}
							if (k >= 12) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%B' -> unknow month = '" + sNumber + "'");
							today.setMonth(k + 1);
							j += iLength;
						}
						break;
					case 't':
						today.setWINGZDate(getDouble(sDate, 300, j));
						break;
					case 'T':
						today.setHour(getInteger(sDate, 2, j));
						if ((today.getHour() < 0) || (today.getHour() > 23)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%T -> %H'");
						if (sDate[j++] != ':') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%T -> ':' expected after %H'");
						today.setMin(getInteger(sDate, 2, j));
						if ((today.getMin() < 0) || (today.getMin() > 59)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%T -> %M'");
						if (sDate[j++] != ':') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%T -> ':' expected after %M'");
						today.setSec(getInteger(sDate, 2, j));
						if ((today.getSec() < 0) || (today.getSec() > 59)) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%T -> %S'");
						break;
					case '|':
						// nothing to do
						break;
					case 'a':
					case 'A':
					case 'e':
					case 'j':
					case 'p':
					case 'r':
					case 'U':
					case 'w':
						throw UtlException("date format \"... " + std::string(sFormat.c_str() + (i-1)) + "' not implemented yet");
					default:
						throw UtlException("syntax error on date format \"" + sFormat + "\" for '%" + sFormat[i] + "'");
				}
			} else j++;
			i++;
		}
		if (i < sFormat.size()) {
			if ((sFormat[i] != '%') || (sFormat[i + 1] != '|')) {
				throw UtlException("date format \"" + sFormat + "\" not applied on \"" + sDate + "\" completely");
			}
		}
		return today;
	}

	void UtlDate::addDateFromFormat(const std::string& sFormat, const std::string& sDate) {
		std::string::size_type i = 0;
		std::string::size_type j = 0;
		std::string sNumber;
		while ((i < sFormat.size()) && (j < sDate.size())) {
			if (sFormat[i] == '%') {
				i++;
				int iSigned = ((sDate[j] == '-') ? 1 : 0);
				switch(sFormat[i]) {
					case 'Y':
						if (j + 4 + iSigned > sDate.size()) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Y'");
						sNumber = sDate.substr(j, 4 + iSigned);
						addYear(atoi(sNumber.c_str()));
						j += 4 + iSigned;
						break;
					case 'y':
						addYear(getInteger(sDate, 2 + iSigned, j));
						break;
					case 'm':
						addMonth(getInteger(sDate, 2 + iSigned, j));
						break;
					case 'd':
						addDay(getInteger(sDate, 2 + iSigned, j));
						break;
					case 'H':
						addHour(getInteger(sDate, 2 + iSigned, j));
						break;
					case 'I':
						if (j + 2 + iSigned > sDate.size()) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%I'");
						sNumber = sDate.substr(j, 2 + iSigned);
						addHour(atoi(sNumber.c_str()));
						j += 2 + iSigned;
						break;
					case 'M':
						addMin(getInteger(sDate, 2 + iSigned, j));
						break;
					case 'S':
						addSec(getInteger(sDate, 2 + iSigned, j));
						break;
					case '%': break;
					case 'D':
						addMonth(getInteger(sDate, 2 + iSigned, j));
						if (sDate[j++] != '/') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%D -> '/' expected after %m'");
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						addDay(getInteger(sDate, 2 + iSigned, j));
						if (sDate[j++] != '/') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%D -> '/' expected after %d'");
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						addYear(getInteger(sDate, 2 + iSigned, j));
						break;
					case 'Q':
						addYear(getInteger(sDate, 4 + iSigned, j));
						if (sDate[j++] != '-') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> '-' expected after %Y'");
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						addMonth(getInteger(sDate, 2 + iSigned, j));
						if (sDate[j++] != '-') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> '-' expected after %m'");
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						addDay(getInteger(sDate, 2 + iSigned, j));
						if (sDate[j++] != ' ') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> ' ' expected after %d'");
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						addHour(getInteger(sDate, 2 + iSigned, j));
						if (sDate[j++] != ':') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> ':' expected after %H'");
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						addMin(getInteger(sDate, 2 + iSigned, j));
						if (sDate[j++] != ':') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%Q -> ':' expected after %M'");
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						addSec(getInteger(sDate, 2 + iSigned, j));
						if (sDate[j] != '.') break;
						j++;
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						// Be careful!
						// Continue in sequence on case '%L'!
					case 'L':
						// Be careful!
						// Don't move this case label and don't insert anything before case '%Q'
						{
							if (iSigned != 0) j++;
							if (j >= sDate.size()) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%L'");
							char a = sDate[j];
							if ((a < '0') || (a > '9')) throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%L'");
							int iMillis = 0;
							int iLoop = 0;
							do {
								iMillis = 10*iMillis + (a - '0');
								a = sDate[++j];
							} while ((++iLoop < 3) && (a >= '0') && (a <= '9'));
							if (iSigned != 0) iMillis = -iMillis;
							addMillis(iMillis);
						}
						break;
					case 't':
						setWINGZDate(getWINGZDate() + getDouble(sDate, 300, j));
						break;
					case 'T':
						addHour(getInteger(sDate, 2 + iSigned, j));
						if (sDate[j++] != ':') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%T -> ':' expected after %H'");
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						addMin(getInteger(sDate, 2 + iSigned, j));
						if (sDate[j++] != ':') throw UtlException("syntax error on date format: format \"" + sFormat + "\" doesn't match with date '" + sDate + "' for '%T -> ':' expected after %M'");
						iSigned = ((sDate[j] == '-') ? 1 : 0);
						addSec(getInteger(sDate, 2 + iSigned, j));
						break;
					case '|':
						// nothing to do
						break;
					case 'a':
					case 'A':
					case 'b':
					case 'B':
					case 'e':
					case 'j':
					case 'p':
					case 'r':
					case 'U':
					case 'w':
						throw UtlException("date format \"... " + std::string(sFormat.c_str() + (i-1)) + "' has no sense here");
					default:
						throw UtlException("syntax error on date format \"" + sFormat + "\" for '%" + sFormat[i] + "'");
				}
			} else j++;
			i++;
		}
		if (i < sFormat.size()) {
			if ((sFormat[i] != '%') || (sFormat[i + 1] != '|')) {
				throw UtlException("date format \"" + sFormat + "\" not applied on \"" + sDate + "\" completely");
			}
		}
	}

	int UtlDate::getDayWINGZDate() const {
		return _pInternalDate->getDayWINGZDate();
	}

	double UtlDate::getWINGZDate() const {
		if (_pInternalDate == NULL) return 0.0;
		return _pInternalDate->getWINGZDate();
	}

	void UtlDate::setWINGZDate(double dWINGZDate) {
		delete _pInternalDate;
		if (dWINGZDate <= 0.0) _pInternalDate = NULL;
		else _pInternalDate = new UtlInternalDate(dWINGZDate);
	}


	int UtlDate::getInteger(const std::string& sValue, int iNbMaxOfDigits, std::string::size_type& iCursor) {
		bool bSigned;
		char a = sValue[iCursor];
		if (a == '-') {
			bSigned = true;
			a = sValue[++iCursor];
			iNbMaxOfDigits--;
		} else {
			bSigned = false;
		}
		if ((a < '0') || (a > '9')) {
			if (bSigned) --iCursor;
			return 0;
		}
		int iNumber = 0;
		do {
			iNumber *= 10;
			iNumber += (int) (a - '0');
			iCursor++;
			iNbMaxOfDigits--;
			if (iCursor >= sValue.size()) break;
			a = sValue[iCursor];
		} while ((a >= '0') && (a <= '9') && (iNbMaxOfDigits > 0));
		if (bSigned) iNumber = -iNumber;
		return iNumber;
	}


	double UtlDate::getDouble(const std::string& sValue, int iNbMaxOfDigits, std::string::size_type& iCursor) {
		bool bSigned;
		char a = sValue[iCursor];
		if (a == '-') {
			bSigned = true;
			a = sValue[++iCursor];
			iNbMaxOfDigits--;
		} else {
			bSigned = false;
		}
		if ((a < '0') || (a > '9')) {
			if (bSigned) --iCursor;
			return 0.0;
		}
		double dNumber = 0.0;
		do {
			dNumber *= 10.0;
			dNumber += (double) ((int) (a - '0'));
			iCursor++;
			iNbMaxOfDigits--;
			if (iCursor >= sValue.size()) return (bSigned) ? -dNumber : dNumber;
			a = sValue[iCursor];
		} while ((a >= '0') && (a <= '9') && (iNbMaxOfDigits > 0));
		if ((a == '.') && (iNbMaxOfDigits > 0)) {
			iCursor++;
			a = sValue[iCursor];
			double dPower = 0.1;
			while ((a >= '0') && (a <= '9') && (iNbMaxOfDigits > 0)) {
				dNumber += dPower * (double) ((int) (a - '0'));
				dPower  *= 0.1;
				iCursor++;
				iNbMaxOfDigits--;
				if (iCursor >= sValue.size()) return dNumber;
				a = sValue[iCursor];
			}
		}
		return (bSigned) ? -dNumber : dNumber;
	}


	int UtlDate::getFrequency() {
		if (_pInternalDate==NULL) {
			return 0;
		}
		else {
			if (_pInternalDate->instanceOf("UtlVariableDate")) {
				UtlVariableDate *pDateCast= (UtlVariableDate *) _pInternalDate;
				return pDateCast->getFrequency();
			}
			else {
				return 0;
			}
			
		}
	}

	char UtlDate::getUnit() {
		if (_pInternalDate==NULL) {
			return 0;
		}
		else {
			if (_pInternalDate->instanceOf("UtlVariableDate")) {
				UtlVariableDate *pDateCast= (UtlVariableDate *) _pInternalDate;
				return pDateCast->getUnit();
			}
			else {
				return 0;
			}
			
		}
	}

	const std::string& UtlDate::getComment() const {
		static std::string sEmpty = "";
		if (_pInternalDate == NULL) return sEmpty;
		return _pInternalDate->getComment();
	}

	void UtlDate::setComment(const std::string& sComment) {
		if (_pInternalDate != NULL) {
			_pInternalDate->setComment(sComment);
		}
	}
}
