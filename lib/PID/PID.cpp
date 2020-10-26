/*
	Setup new PID function that works in the same way that 
*/

#include "PID.h"

void PID::initialize(int pwm_pin, double targetTemp, double actualTemp){
	ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcAttachPin(pwm_pin, PWM_CHANNEL);
}

void PID::tune(double kp, double ki, double kd){
	this->kp = kp;
	this->ki = ki;
	this->kd = kd;
}

void PID::compute(double targetTemp, double actualTemp){
	double error = targetTemp - actualTemp;
	double dErr = error - lastErr;

	P = kp  * error;
	if(P > MAX_TERM_VALUE) P = MAX_TERM_VALUE;
	else if(P < -1 * MAX_TERM_VALUE) P = -1 * MAX_TERM_VALUE;

	I = I + (ki * error);
	if(I > MAX_TERM_VALUE) I = MAX_TERM_VALUE;
	else if(I < -1 * MAX_TERM_VALUE) I = -1 * MAX_TERM_VALUE;

	D =  kd * dErr;
	if(D > MAX_TERM_VALUE) D = MAX_TERM_VALUE;
	else if(D < -1 * MAX_TERM_VALUE) D = -1 * MAX_TERM_VALUE;

	lastErr = error;
	double pV = P + I + D;
	Serial.println(millis());
	Serial.printf("Error: %f Rate of Error: %f P: %f I: %f D: %f \n", error, dErr, P, I, D);

	if(pV < 0) pV = 0;
	else if(pV > MAX_TERM_VALUE) pV = MAX_TERM_VALUE;

	double output =  ( pV / 1000 ) * MAX_HEATER_POWER;
	Serial.printf("TargetTemp: %f Temp: %f OutputV: %f\n", targetTemp, actualTemp, output);

	ledcWrite(PWM_CHANNEL, output);
}