/*
	Setup new PID function that works in the same way that 
*/

#include "PID.h"

PID::PID(double* input, double *output, double *setpoint){
	this->lastTime = millis() - sampleTime;
}

void PID::tune(double kp, double ki, double kd){
	this->kp = kp;
	this->ki = ki;
	this->kd = kd;
}

void PID::compute(){
	ulong now = millis();
	ulong timeChange = now - lastTime;
	if(timeChange >= sampleTime) {
		double input = *(this->input);
		double error = *(this->setpoint) - input;
		double dInput = input - lastInput;
		outputSum += (ki * error);

		if(outputSum > cycleTime) outputSum = cycleTime;
		else if (outputSum < 0) outputSum = 0;

		double result = 0;
		result += outputSum - kd * dInput;

		if(outputSum > cycleTime) outputSum = cycleTime;
		else if (outputSum < 0) outputSum = 0;

		*(this->output) = result;

		lastInput = input;
		lastTime = now;
	}
}