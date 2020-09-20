/*
	Setup new PID function that works in the same way that 
*/

#include "PID.h"

void PID::initialize(float targetTemp, float actualTemp){
	this->heater_status = true;
}

void PID::compute(float targetTemp, float actualTemp) {
	//find ultimate gain by recording the last time we got above sp, and how long the curve was above sp
	if(targetTemp > actualTemp && heater_status) {
		this->lastTime = millis();
		this->lastErr = targetTemp - actualTemp;
		this->heater_status = true;
	}
	if(targetTemp < actualTemp && !heater_status) {
		this->heater_status = false;
		tu = millis() - this->lastTime;
	}

	//if new tu is within 5% of the last tu, then we're considered stable. set ku to the difference
	float error = targetTemp - actualTemp;
	if(((lastErr * .95) - tu) > 0 && ((lastErr * 1.05) + tu) < 0){
		ku = error;
	}
	float pval = 1;
	
	kp = pval * ku;
	if( ti > 0) {
		ki = kp / ti;
	}
	kd = kp * td;
	float dError  = actualTemp - lastTemp;
	errSum = errSum + error;
	outputV = (kp * error) + (ki * errSum) + (kd * dError);

	PWM(outputV);
}

bool PID::isHeaterRunning() {
	return heater_status;
}

/* 
	PWM process plant that varies the wattage down to 
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