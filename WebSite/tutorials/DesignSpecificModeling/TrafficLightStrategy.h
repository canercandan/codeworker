#ifndef _TrafficLightStrategy_h_
#define _TrafficLightStrategy_h_

#include <string>

class TrafficLightStrategy {
	public:
		inline TrafficLightStrategy() : bActive_(false) {}
		virtual ~TrafficLightStrategy();

		virtual bool start() const = 0;
		virtual int executeRules() = 0;

	protected:
		bool bActive_;

		static int time();
		static int vehicles_hour(const std::string& sStreet, const std::string& sCrossroad, const std::string& sDirection);
		static void setDuration(const std::string& sStreet, const std::string& sCrossroad, const std::string& sDirection, const int[] tiDurations);
		static bool activateStrategy(const std::string& sStrategy);
};

#endif
