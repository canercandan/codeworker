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

#ifndef _UtlDate_h_
#define _UtlDate_h_

#include <string>

namespace CodeWorker {
	class UtlInternalDate;


	class UtlDate {
	private:
		UtlInternalDate* _pInternalDate;

	public:
		UtlDate();
		UtlDate(const UtlDate& date);
		UtlDate(double dWINGZDate);
		UtlDate(int iWinfrontDate);
		UtlDate(int iDay, int iMonth, int iYear, int iHour = 0, int iMin = 0, int iSec = 0, int iMillis = 0);
		UtlDate(int iFrequency, char cUnit);

		~UtlDate();

		bool isNull() const;
		void nullify();
		bool isInfinite() const;
		void infinity();

		UtlDate* copyInstance() const;
		UtlDate& operator =(const UtlDate& date);
		bool operator <(const UtlDate& date) const;
		bool operator <=(const UtlDate& date) const;
		bool operator ==(const UtlDate& date) const;
		bool operator !=(const UtlDate& date) const;
		bool operator >=(const UtlDate& date) const;
		bool operator >(const UtlDate& date) const;

		double operator -(const UtlDate& date) const;

		void today();

		bool isAFixedDate() const;
		bool isAVariableDate() const;

		int getFrequency(); 
		char getUnit(); 
		const std::string& getComment() const;
		void setComment(const std::string& sComment);

		int getDay() const;
		const char* getWeekDay() const;
		int getWeekDayOrder() const;
		int getMonth() const;
		int getYear() const;

		void setDay(int iDay);
		void setMonth(int iMonth);
		void setYear(int iYear);

		void addDay(int iDay);
		void addMonth(int iMonth);
		void addYear(int iYear);

		int getHour() const;
		int getMin() const;
		int getSec() const;
		int getMillis() const;

		bool isLastDayOfMonth() const;
		void setLastDayOfMonth();

		void ignoreHours();
		void setTime(int iHour, int iMin, int iSec, int iMillis = 0);
		int compareTime(const UtlDate& date) const;
		void setHour(int iHour);
		void setMin(int iMin);
		void setSec(int iSec);
		void setMillis(int iMillis);

		void addHour(int iHour);
		void addMin(int iMin);
		void addSec(int iSec);
		void addMillis(int iMillis);
		void addDateFromFormat(const std::string& sFormat, const std::string& sDate);

		std::string getString() const;
		std::string getMCLDate() const;
		std::string getFormattedDate(const std::string& sFormat) const;
		static UtlDate getDateFromFormat(const std::string& sDate, const std::string& sFormat);
		int getDayWINGZDate() const;
		double getWINGZDate() const;
		void setWINGZDate(double dWINGZDate);

	private:
		static int getInteger(const std::string& sValue, int iNbMaxOfDigits, std::string::size_type& iCursor);
		static double getDouble(const std::string& sValue, int iNbMaxOfDigits, std::string::size_type& iCursor);
	};
}

#endif
