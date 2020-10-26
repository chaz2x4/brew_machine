#include "gcp.h"

GCP::GCP() {
	GCP(DEFAULT_TARGET_TEMP);
}

GCP::GCP(float targetTemp) {
	init(targetTemp);
}

void GCP::init(){
	init(DEFAULT_TARGET_TEMP);
}

void GCP::init(float targetTemp){
	this->tempProbe.begin(MAX31865_3WIRE);
	this->setTargetTemp(targetTemp);

	ledcSetup(PWM_BREW_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcAttachPin(HEATER_PIN, PWM_BREW_CHANNEL);
	ledcSetup(PWM_STEAM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcAttachPin(STEAM_PIN, PWM_STEAM_CHANNEL);
}

GCP::~GCP() {
	float targetTemp = this->getTargetTemp(); //get current target temp
	if (targetTemp > MAX_BREW_TEMP) targetTemp = DEFAULT_TARGET_TEMP; //if it's higher than max value, then set it to default value of 95.0
};

void GCP::setTargetTemp(float temp) {
	setTargetTemp(temp, MIN_BREW_TEMP, MAX_BREW_TEMP); //Set max default brew temperature to between 90 - 96 degrees
}

void GCP::setTargetTemp(float temp, float minTemp, float maxTemp) {
	if (temp < minTemp) temp = minTemp;
	else if (temp > maxTemp) temp = maxTemp;
	this->targetTemp = temp;
}

void GCP::incrementTemp() {
	float temp = this->getTargetTemp();
	this->setTargetTemp(temp + 0.5);
}

void GCP::decrementTemp() {
	float temp = this->getTargetTemp();
	this->setTargetTemp(temp - 0.5);
}

float GCP::getTargetTemp() {
	return this->targetTemp;
}

float GCP::getPX() {
	return this->pressure;
}

float GCP::getActualTemp() {
	float temp =  tempProbe.temperature(100, RREF);
	uint8_t fault = tempProbe.readFault();
	if (fault) {
		Serial.print("Fault 0x"); Serial.println(fault, HEX);
		if (fault & MAX31865_FAULT_HIGHTHRESH) Serial.println("RTD High Threshold");
		if (fault & MAX31865_FAULT_LOWTHRESH) Serial.println("RTD Low Threshold"); 
		if (fault & MAX31865_FAULT_REFINLOW) Serial.println("REFIN- > 0.85 x Bias");
		if (fault & MAX31865_FAULT_REFINHIGH) Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
		if (fault & MAX31865_FAULT_RTDINLOW) Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
		if (fault & MAX31865_FAULT_OVUV) Serial.println("Under/Over voltage");
		tempProbe.clearFault();
	}
	return temp;
}

void GCP::update() {
	float targetTemp = this->getTargetTemp();
	float actualTemp = this->getActualTemp();
	float pressure = this->getPX();

	double brew_output = brewTempManager.compute(targetTemp, actualTemp);
	double steam_output = steamTempManager.compute(TARGET_STEAM_TEMP, actualTemp);

	Serial.printf("\nActual Temp: %f\n", actualTemp);
	Serial.printf("Brew  Temp: %f PWM %%: %f\n", targetTemp, brew_output * 100);
	Serial.printf("Steam Temp: %f PWM %%: %f\n", TARGET_STEAM_TEMP, steam_output * 100);
	/* 
		Brew Relay and Steam Relay will always be calculating
		When power switch is on the heater will heat until it gets to targetBrewtemp
		Brew Lamp will turn on once the brew relay is off (once target temp is reached)

		If steam switch is pushed then the brew lamp is turned off
		Power defaults to steam relay, brew relay is off
		Steam lamp turn on when steam relay is off

		This is all handled on the hardware side.

		If temperature rises above maixmum safe temperature (10C above steam temp) turn off relay

	*/

	/****** PWM IS A WORK IN PROGRESS ******/
	ledcWrite(PWM_BREW_CHANNEL, brew_output * PWM_DUTY_CYCLE);
	ledcWrite(PWM_STEAM_CHANNEL, steam_output * PWM_DUTY_CYCLE);

	if(actualTemp >= EMERGENCY_SHUTOFF_TEMP) digitalWrite(STEAM_PIN, OFF);
}

