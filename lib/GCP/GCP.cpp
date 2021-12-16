#include "GCP.h"

void GCP::start() {
	init(targetTemp = DEFAULT_BREW_TEMP, targetSteamTemp = DEFAULT_STEAM_TEMP, tempOffset = DEFAULT_OFFSET);
}

void GCP::init(double targetTemp, double targetSteamTemp, double offset) {
	this->tempProbe.begin(MAX31865_3WIRE);
	this->setTargetTemp(targetTemp);
	this->setTargetSteamTemp(targetSteamTemp);
	this->setTempOffset(offset);
	this->actualTemp = this->getActualTemp();

	pinMode(HEATER_PIN, OUTPUT);
	pinMode(STEAM_PIN, OUTPUT);

	brewTempManager.SetMode(AUTOMATIC);
	steamTempManager.SetMode(AUTOMATIC);
	brewTempManager.SetOutputLimits(0, CYCLE_TIME);
	steamTempManager.SetOutputLimits(0, CYCLE_TIME);
}

void GCP::setTargetTemp(double temp) {
	if (temp < minBrewTemp) temp = minBrewTemp;
	else if (temp > maxBrewTemp) temp = maxBrewTemp;
	this->targetTemp = temp;
}

void GCP::setTargetSteamTemp(double temp) {
	if (temp < minSteamTemp) temp = minSteamTemp;
	else if (temp > maxSteamTemp) temp = maxSteamTemp;
	this->targetSteamTemp = temp;
}

void GCP::incrementTemp(String currentMode) {
	double temp;
	double i = 0.5;
	if(currentMode == "steam") { 
		temp = targetSteamTemp;
		i = 1;
	}
	else temp = targetTemp;
	temp += i;
	if(currentMode == "steam") this->setTargetSteamTemp(temp);
	else this->setTargetTemp(temp);
}

void GCP::decrementTemp(String currentMode) {
	double temp;
	double i = 0.5;
	if(currentMode == "steam") {
		temp = targetSteamTemp;
		i = 1;
	}
	else temp = targetTemp;
	temp -= i;
	if(currentMode == "steam") this->setTargetSteamTemp(temp);
	else this->setTargetTemp(temp);
}

double GCP::getTargetTemp() {
	return this->targetTemp;
}

double GCP::getTargetSteamTemp() {
	return this->targetSteamTemp;
}

double GCP::getActualTemp() {
	double temp =  tempProbe.temperature(100, RREF);
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
	temp += tempOffset;
	this->actualTemp = temp;
	return temp;
}

double GCP::getTempOffset(){
	return this->tempOffset;
}

void GCP::setTempOffset(double offset){
	if(offset > MAX_OFFSET) this->tempOffset = MAX_OFFSET;
	else if(offset < MIN_OFFSET) this->tempOffset = MIN_OFFSET;
	else this->tempOffset = offset;
}

String GCP::getOutput(){
	return this->outputString;
}

String GCP::getTunings(String currentMode){
	String output;
	PID *tempManager;
	if(currentMode == "steam") tempManager = &steamTempManager;
	else tempManager = &brewTempManager;
    output += "{ \"kp\": ";
    output += tempManager->GetKp();
    output += ", \"ki\": ";
    output += tempManager->GetKi();
    output +=  ", \"kd\": ";
    output += tempManager->GetKd();
    output += " }";
    return output;
}

void GCP::setTunings(String currentMode, double kp, double ki, double kd){
	if(currentMode == "steam") steamTempManager.SetTunings(kp, ki, kd, P_ON_M);
	else brewTempManager.SetTunings(kp, ki, kd, P_ON_M);
}

void GCP::parseQueue(ulong time){
	String outputs;
    outputs += "{ \"time\": ";
    outputs += time;
    outputs += ", \"temperature\": ";
    outputs += this->getActualTemp();
    outputs += ", \"outputs\": { \"brew\": ";
    outputs += this->brew_output;
    outputs += ", \"steam\": ";
    outputs += this->steam_output;
    outputs += " }}";
	outputQueue.push(outputs);

	outputString = "{ \"targets\": { \"brew\": ";
	outputString += this->targetTemp;
	outputString += ", \"steam\": ";
	outputString += this->targetSteamTemp;
	outputString += ", \"offset\": ";
	outputString += this->tempOffset;
	outputString += " }, \"outputs\":[";

	for(unsigned i = 0; i < outputQueue.size(); i++){
		if(i > 0 ) outputString += ",";
		outputString += outputQueue.at(i);
	}

	outputString += "]}";
}

void GCP::refresh(ulong realTime) {
	/* 
		Brew Relay and Steam Relay will always be calculating
		When power switch is on the heater will heat until it gets to targetBrewtemp
		Brew Lamp will turn on once the brew relay is off (once target temp is reached)

		If steam switch is pushed then the brew lamp is turned off
		Power defaults to steam relay, brew relay is off
		Steam lamp turn on when steam relay is off

		This is all handled on the hardware side.

		If temperature rises above maximum safe temperature turn off relay
	*/

	ulong runTime = millis();
	if(runTime - cycleStartTime > CYCLE_TIME) {
		parseQueue(realTime);
		cycleStartTime += CYCLE_TIME;
		brewTempManager.Compute();
		steamTempManager.Compute();
	}
	
	if(brew_output > runTime - cycleStartTime) digitalWrite(HEATER_PIN, ON);
	else digitalWrite(HEATER_PIN, OFF);

	if(steam_output > runTime - cycleStartTime) digitalWrite(STEAM_PIN, ON);
	else digitalWrite(STEAM_PIN, OFF);
	
	double actualTemp = this->getActualTemp();
	if((actualTemp + tempOffset) >= EMERGENCY_SHUTOFF_TEMP) {
		digitalWrite(HEATER_PIN, OFF);
		digitalWrite(STEAM_PIN, OFF);
	}
}