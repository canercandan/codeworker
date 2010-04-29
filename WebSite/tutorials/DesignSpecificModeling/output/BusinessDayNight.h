#ifndef _BusinessDayNight_h_
#define _BusinessDayNight_h_

#include "TrafficLightStrategy.h"

class BusinessDayNight : public TrafficLightStrategy {
	public:
		inline BusinessDayNight() {}
		virtual ~BusinessDayNight();

		virtual bool start() const;		
		virtual int executeRules();

	private:
		bool executeRule0();
		bool executeRule1();
		bool executeRule2();
		bool executeRule3();
};

#endif
