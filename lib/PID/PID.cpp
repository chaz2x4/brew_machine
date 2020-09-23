/*
	Setup new PID function that works in the same way that 
*/

#include "PID.h"

void PID::initialize(int pwm_pin, float targetTemp, float actualTemp){
	ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcAttachPin(pwm_pin, PWM_CHANNEL);
	this->fullDutyCycle = pow(2, PWM_RESOLUTION);
}

void PID::compute(float targetTemp, float actualTemp){
	float error = targetTemp - actualTemp;
	float dErr = lastErr - error;
	float outputV = fullDutyCycle; //Set heater to maximum value initially to get things started

	//Find ku
	if(millis() - lastCycleTime >= DUTY_PERIOD) {
		//Record the value of proportional gain and check if it matches the last ultimate gain to determine oscillations
		if(actualTemp > targetTemp) {
			if(kp > 0 && tu == 0) {
				if(kp == lastKp && (abs(period - lastPeriod) < 1000)) {
					tu = period;
					ku = kp;
				}
				lastPeriod = period;
				period = millis() - lastTime;
				lastKp = kp;
				kp = 0;
				lastTime = millis();
			}
			outputV = 0;
		}
		Serial.printf("Kp: %f lastKp: %f period: %lu lastPeriod: %lu ", kp, lastKp, period, lastPeriod);
		if(actualTemp < targetTemp && tu == 0) kp = kp + 0.05; //slow increase proportional gain on every duty cycle; Results in 5 seconds to get to maximum heater output at an error of 1.0
	}

	//Use the Ziegler Nichols Classic PID calculations
	if(ku > 0 && tu > 0) {
		float ki = 0.0;
		float kd = 0.0;
		float ti =  tu / 2.0;
		float td =  tu / 8.0;

		kp = (3 * ku) / 5.0;
		if ( ti > 0 ) ki = kp / ti;
		kd = kp * td;

		lastErr = error;
		errSum = errSum + error;

		outputV = (kp * error) + (ki * errSum) + (kd * dErr);
	}

	Serial.printf("TargetTemp: %f Temp: %f OutputV: %f\n", targetTemp, actualTemp, outputV);

	if(outputV > fullDutyCycle) outputV = fullDutyCycle;
	if(outputV < 0) outputV = 0;

	ledcWrite(PWM_CHANNEL, outputV);
}