#include "gcp.h"

GCP::GCP() {
	GCP(DEFAULT_TARGET_TEMP);
}

GCP::GCP(float targetTemp) {
	init(targetTemp);
}

void GCP::init(float targetTemp){
	this->tempProbe.begin(MAX31865_3WIRE);
	this->power_light = ON;
	this->setTargetTemp(targetTemp);
	float actualTemp = this->getActualTemp();
	this->temperatureManager.initialize(targetTemp, actualTemp);
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
	return temp;
}

bool GCP::isSteamReady() {
	return this->steam_light;
}

bool GCP::isBrewReady() {
	return this->brew_light;
}

bool GCP::isPowerOn() {
	return this->power_light;
}

void GCP::update() {
	float targetTemp = this->getTargetTemp();
	float actualTemp = this->getActualTemp();

	temperatureManager.compute(targetTemp, actualTemp);
	this->heating_switch = temperatureManager.isHeaterRunning();

	//Turn on lights if within 1 degree C of target temp
	float dtemp = targetTemp - actualTemp; //temperature difference
	if (dtemp >= -1.0 || dtemp <= 1.0) {
		bool steam_switch = this->steam_switch;
		if (steam_switch) {
			this->steam_light = ON; 
			this->brew_light = OFF;
		}
		else { 
			this->brew_light = ON; 
			this->steam_light = OFF;
		}
	}
	else {
		this->steam_light = OFF;
		this->brew_light = OFF;
	}

	if(actualTemp >= EMERGENCY_SHUTOFF_TEMP) {
		heating_switch = OFF;
	}

	digitalWrite(HEATER_PIN, heating_switch);
	digitalWrite(STEAM_LIGHT_PIN, steam_light);
	digitalWrite(BREW_LIGHT_PIN, brew_light);
	digitalWrite(POWER_LIGHT_PIN, power_light);
}
