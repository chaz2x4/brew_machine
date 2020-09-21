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
		if(actualTemp < targetTemp && tu == 0) kp = kp + 0.1; //slow increase proportional gain on every duty cycle; Results in 5 seconds to get to maximum heater output at an error of 1.0
		Serial.printf("Kp: %f Ku: %f periodTime: %lu Tu: %lu Temp: %f\n", kp, ku, periodTime, tu, actualTemp);
		lastTemp = actualTemp;
	}

	if(tu > 0) {
		//Use the Pessen Integration Rule to determine ti and td
		if(kp == 0) tu = 0;
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

	if(outputV > fullDutyCycle) outputV = fullDutyCycle;
	if(outputV < 0) outputV = 0;

	ledcWrite(PWM_CHANNEL, outputV);
}