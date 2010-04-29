package org.trafficlights.simulator;

import org.trafficlights.rules.*;

// All legacy strategies, coming from ".tlc" files, are injected here
// in Java, as inner classes.
//##markup##"strategies"
//##begin##"strategies"
class BusinessDayNight extends TrafficLightStrategy {
	public BusinessDayNight() {}

	public bool start() {
		return time() > (17*60 + 30)*60;
	}

	public int executeRules() {
		int iTriggeredRules = 0;
		if (bActive_) {
			if (executeRule0()) ++iTriggeredRules;
			if (executeRule1()) ++iTriggeredRules;
			if (executeRule2()) ++iTriggeredRules;
			if (executeRule3()) ++iTriggeredRules;
		}
		return iTriggeredRules;
	}
	
	private bool executeRule0() {
		if (vehicles_hour("boulevard des Capucines", "place_opera", "c->s") < 400 == false) {
			return false;
		}
		{
			int[] durations = {65, 120, -1};
			setDuration("rue Scribe", "scribe", "c->s", durations);
		}
		return true;
	}
	
	private bool executeRule1() {
		if (vehicles_hour("rue de la Paix", "rue_paix", "s->c") > 300 == false) {
			return false;
		}
		activateStrategy("RivoliLowDensity");
		return true;
	}
	
	private bool executeRule2() {
		if (vehicles_hour("rue auber", "auber", "c->s") > 500 == false) {
			return false;
		}
		bActive_ = false;
		return true;
	}
	
	private bool executeRule3() {
		if (time() > (22*60 + 30)*60 == false) {
			return false;
		}
		bActive_ = false;
		return true;
	}
	
}

//##end##"strategies"

/*
Here, the developer will directly type some strategies, which aren't
the legacy one, as specified by the customer in ".tlc" files, but those
// that he wants to test before trashing them.

The DSL code inside the ##data## tags isn't Java valid code, so one encloses
the complete markup between comments
//##markup##"DSL: TrafficLight"
//##data##
strategy BusinessDayMorning {
	start time() < (6*60 + 30)*60; // activate the rule before 6h30
	
	vehicles_hour("avenue de l'Opera"->place_opera) > 800
		=> duration("boulevard des Italiens"->place_opera, 0min50/1min10);
	vehicles_hour("boulevard des Capucines"->place_opera) > 700
		=> duration(scribe->"rue Scribe", 1min05/1min30);
	time() > (9*60 + 30)*60 => desactivate; // inhibate the rule after 9h30
}
//##data##
//##begin##"DSL: TrafficLight"
*/
class BusinessDayMorning extends TrafficLightStrategy {
	public BusinessDayMorning() {}

	public bool start() {
		return time() < (6*60 + 30)*60;
	}

	public int executeRules() {
		int iTriggeredRules = 0;
		if (bActive_) {
			if (executeRule0()) ++iTriggeredRules;
			if (executeRule1()) ++iTriggeredRules;
			if (executeRule2()) ++iTriggeredRules;
		}
		return iTriggeredRules;
	}
	
	private bool executeRule0() {
		if (vehicles_hour("avenue de l'Opera", "place_opera", "s->c") > 800 == false) {
			return false;
		}
		{
			int[] durations = {50, 70, -1};
			setDuration("boulevard des Italiens", "place_opera", "s->c", durations);
		}
		return true;
	}
	
	private bool executeRule1() {
		if (vehicles_hour("boulevard des Capucines", "place_opera", "s->c") > 700 == false) {
			return false;
		}
		{
			int[] durations = {65, 90, -1};
			setDuration("rue Scribe", "scribe", "c->s", durations);
		}
		return true;
	}
	
	private bool executeRule2() {
		if (time() > (9*60 + 30)*60 == false) {
			return false;
		}
		bActive_ = false;
		return true;
	}
	
}

/*
//##end##"DSL: TrafficLight"
*/

public class Simulator {
	// This class is implemented by hand and works on strategies
	// previously declared as inner classes.
	// ...
}
