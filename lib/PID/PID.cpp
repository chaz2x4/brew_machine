/*
	Setup new PID function that works in the same way that 
*/


#include "PID.h"

#define MAX_OUTPUT 100
#define MIN_OUTPUT 0

void PID::compute(int sp, float pv) {
	int targetTemp = sp;
	float actualTemp = pv;

	unsigned long now = millis();
	int timeChange = (now - this->lastTime);

	if (timeChange >= runTime) {
		float error = targetTemp - actualTemp; //Error = setPoint - processVariable e(t) = SP-PV(t). Difference between set temperature and actual temperature
		float dError = actualTemp - this->lastTemp; // de(t)/dt = change in error. Difference between last temperature at last runtime and current temperature

		float P = error * this->kp;
		float I = this->iTerm + (this->ki * error); // integral e(T)dT = sum of errors up to this point. 
		float D = dError * this->kd;

		if (I > MAX_OUTPUT) I = MAX_OUTPUT;
		else if (I < MIN_OUTPUT) I = MIN_OUTPUT;

		float outputV = P + I - D;

		if (outputV > MAX_OUTPUT) outputV = MAX_OUTPUT;
		else if (outputV < MIN_OUTPUT) outputV = MIN_OUTPUT;

		this->outputV = outputV;
		this->lastTime = now;
		this->lastTemp = actualTemp;
		this->iTerm = I;
	}
}

void PID::setTunings(float kp, float ki, float kd) {
	int runTime = this->runTime;
	this->kp = kp;
	this->ki = ki * runTime;
	this->kd = kd / runTime;
}

void PID::setRuntime(int runTime) {
	if (runTime > 0) {
		float ratio = ((float) runTime) / 1000; //convert ki and kd to new runtime so that they don't give whacky numbers
		this->ki = this->ki * ratio;
		this->kd = this->kd / ratio;
		this->runTime = runTime;
	}
}