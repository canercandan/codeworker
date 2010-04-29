#include "BusinessDayNight.h"

BusinessDayNight::~BusinessDayNight() {}

bool BusinessDayNight::start() const {
	return time() > (17*60 + 30)*60;
}

int BusinessDayNight::executeRules() {
	int iTriggeredRules = 0;
	if (bActive_) {
		if (executeRule0()) ++iTriggeredRules;
		if (executeRule1()) ++iTriggeredRules;
		if (executeRule2()) ++iTriggeredRules;
		if (executeRule3()) ++iTriggeredRules;
//##protect##"Post Processing, to handle by hand!"
		// hand-typed code, added by CL
		if (iTriggeredRules > 3) setHigherPriority();
//##protect##"Post Processing, to handle by hand!"
	}
	return iTriggeredRules;
}

bool BusinessDayNight::executeRule0() {
	if (!bActive_ || (vehicles_hour("boulevard des Capucines", "place_opera", "c->s") < 400 == false)) return false;
	{
		int durations[] = {65, 120, -1};
		setDuration("rue Scribe", "scribe", "c->s", durations);
	}
	return true;
}

bool BusinessDayNight::executeRule1() {
	if (!bActive_ || (vehicles_hour("rue de la Paix", "rue_paix", "s->c") > 300 == false)) return false;
	activateStrategy("RivoliLowDensity");
	return true;
}

bool BusinessDayNight::executeRule2() {
	if (!bActive_ || (vehicles_hour("rue auber", "auber", "c->s") > 500 == false)) return false;
	bActive_ = false;
	return true;
}

bool BusinessDayNight::executeRule3() {
	if (!bActive_ || (time() > (22*60 + 30)*60 == false)) return false;
	bActive_ = false;
	return true;
}

