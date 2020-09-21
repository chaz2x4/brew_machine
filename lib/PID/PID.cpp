/*
	Setup new PID function that works in the same way that 
*/

#include "PID.h"

void PID::initialize(float targetTemp, float actualTemp){
	this->heater_status = true;
}

void PID::compute(float targetTemp, float actualTemp){
	float error = targetTemp - actualTemp;
	float dErr = lastErr - error;
	float outputV = 1; //Set heater to maximum value initially to get things started

	//Find ku
	if(millis() - lastCycleTime >= DUTY_CYCLE) {
		//Record the value of proportional gain and check if it matches the last ultimate gain to determine oscillations
		if(actualTemp > targetTemp) {
			if(kp > 0 && tu == 0) {
				if(kp == ku) {
					if(millis() - lastTime == periodTime) {
						tu = periodTime;
					}
					periodTime = millis() - lastTime;
				}
				ku = kp;
				kp = 0; //if stable oscillation isn't found then reset proportional gain
				lastTime = millis();
			}
		}
		if(actualTemp < targetTemp && tu == 0) kp = kp + 0.01; //slow increase proportional gain on every duty cycle; Results in 5 seconds to get to maximum heater output at an error of 1.0
		Serial.printf("Kp: %f Ku: %f periodTime: %lu Tu: %lu Temp: %f\n", kp, ku, periodTime, tu, actualTemp);
		lastTemp = actualTemp;
	}

	if(tu > 0) {
		//Use the Pessen Integration Rule to determine ti and td
		if(kp = 0) tu = 0;
		float ki = 0.0;
		float kd = 0.0;
		float ti = (2 * tu) / 5.0;
		float td = (3 * tu) / 20.0;

		kp = (7 * ku) / 10.0;
		if ( ti > 0 ) ki = kp / ti;
		kd = kp * td;

		lastErr = error;
		errSum = errSum + error;

		outputV = (kp * error) + (ki * errSum) + (kd * dErr);
	}

	PWM(outputV);
}

bool PID::isHeaterRunning(){
	return heater_status;
}

/* 
	PWM process plant that varies the wattage
	We'll use a 0.5 second cycle which should result in a 1.666% resolution
*/
void PID::PWM(float powerPercent){
	if(powerPercent > 1.0) powerPercent = 1;
	if(powerPercent < 0.0) powerPercent = 0;
	long onTime = DUTY_CYCLE * powerPercent;
	if(millis() - lastCycleTime >= DUTY_CYCLE) {
		this->lastCycleTime = millis();
		this->heater_status = true;
	}
	if(millis() - lastCycleTime >= onTime) {
		this->heater_status = false;
	}
}