/*
	Setup new PID function that works in the same way that 
*/

#include "Controller.h"

Controller::Controller(double* input, double *output, double *setpoint, ulong time){
	lastTime = millis() - sampleTime;
	currentTemp = input;
	onTime = output;
	targetTemp = setpoint;
	cycleRunTime = time;
}

void Controller::tune(double kp, double ki, double kd){
	this->kp = kp;
	this->ki = ki;
	this->kd = kd;
}

double* Controller::getTunings(double* tunings){
	tunings[0] = this->kp;
	tunings[1] = this->ki;
	tunings[2] = this->kd;
	return tunings;
}

void Controller::compute(){
	ulong now = millis();
	ulong timeChange = now - lastTime;
	if(timeChange >= sampleTime) {
		double input = *currentTemp;
		double error = *targetTemp - input;
		double dInput = input - lastInput;
		outputSum += (ki * error);
		outputSum -= (kp * dInput);

		if(outputSum > cycleRunTime) outputSum = cycleRunTime;
		else if (outputSum < 0) outputSum = 0;

		double output = 0;
		output += outputSum - kd * dInput;

	    if(output > cycleRunTime) output = cycleRunTime;
      	else if(output < 0) output = 0;

		*onTime = output;

		lastInput = input;
		lastTime = now;
	}
}

void Controller::setCycleTime(double time){
	this->cycleRunTime = time;
}