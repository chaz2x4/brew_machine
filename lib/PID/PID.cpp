/*
	Setup new PID function that works in the same way that 
*/

#include "PID.h"

void PID::initialize(float targetTemp, float actualTemp){
	runTime = (BOILER_SIZE * (targetTemp - actualTemp) * CP_WATER / HEATER_POWER) / EFFICIENCY; // initially set the boiler to heat for a minimum of this many seconds
	Serial.printf("target temp: %.1f; actual temp: %.1f; time to heat: %i\n", targetTemp, actualTemp, runTime);
}

void PID::compute(float targetTemp, float actualTemp) {
	//find ultimate gain
	unsigned long now = millis();
	int timeChange = (now - this->lastTime);

	float pval = 0.05;

	if (timeChange >= runTime) {
		kp = pval * ku;
		if( ti > 0) {
			ki = kp / ti;
		}
		kd = kp * td;

		float error = targetTemp - actualTemp;
		float dError  = actualTemp - lastTemp;

		errSum = errSum + error;
		outputV = (kp * error) + (ki * errSum) + (kd * dError);
	}
}

bool PID::runHeater() {
	return heating_element;
}

void PID::setTunings(float ku, int tu) {
	this->ku = ku;
	this->tu = tu;
}

void PID::setRuntime(int runTime) {
	if (runTime > 0) {
		float ratio = ((float) runTime) / 1000; //convert ki and kd to new runtime so that they don't give whacky numbers
		this->ki = this->ki * ratio;
		this->kd = this->kd / ratio;
		this->runTime = runTime;
	}
}