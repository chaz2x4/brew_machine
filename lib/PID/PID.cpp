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
	float ki = 0.0;
	float kd = 0.0;

	//Use the Pessen Integration Rule to determine ti and td
	float kp = (7 * ku) / 10.0;
	float ti = (2 * tu) / 5.0;
	float td = (3 * tu) / 20.0;

	if ( ti > 0 ) ki = kp / ti;
	kd = kp * td;

	lastErr = error;
	lastTemp = actualTemp;
	errSum = errSum + error;

	float outputV = (kp * error) + (ki * errSum) + (kd * dErr);
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
	long onTime = DUTY_CYCLE * powerPercent;
	if(millis() - lastCycleTime >= DUTY_CYCLE) {
		lastCycleTime = millis();
		this->heater_status = true;
	}
	if(millis() - lastCycleTime >= onTime) {
		this->heater_status = false;
	}
}