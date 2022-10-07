#include "gcp.h"

GCP::GCP()
: tempProbe(Adafruit_MAX31865(THERMOPROBE_PIN))
, kEmergencyShutoffTemp(165.0)
, kMaxBrewTemp(105.0)
, kMinBrewTemp(85.0)
, kMaxSteamTemp(150.0)
, kMinSteamTemp(130.0)
, kMaxOffset(11)
, kMinOffset(-11)
, kWindowSize(1000)
, kLogInterval(500)
, kPowerFrequency(60)
, temp_offset(-8)
, target_temp(92)
, target_steam_temp(140)
, preinfusion_time(2000)
, kp(40)
, ki(6.67)
, kd(52.27)
, brewTempManager(QuickPID(&current_temp, &brew_output, &target_temp))
, steamTempManager(QuickPID(&current_temp, &steam_output, &target_steam_temp))
, outputQueue(Queue(60000 / kLogInterval, C))
{}

GCP::~GCP(){
	digitalWrite(HEATER_PIN, LOW);
	digitalWrite(STEAM_PIN, LOW);
}

void GCP::start() {
	loadParameters();

	this->tempProbe.begin(MAX31865_3WIRE);
	this->getCurrentTemp();

	pinMode(HEATER_PIN, OUTPUT);
	pinMode(STEAM_PIN, OUTPUT);

	brewTempManager.SetTunings(kp, ki, kd);
	steamTempManager.SetTunings(kp, ki, kd);

	brewTempManager.SetOutputLimits(0, kWindowSize);
	steamTempManager.SetOutputLimits(0, kWindowSize);

	brewTempManager.SetMode(brewTempManager.Control::automatic);
	steamTempManager.SetMode(steamTempManager.Control::automatic);
}

void GCP::setTargetTemp(TempMode current_mode, float temp) {
	float minTemp;
	float maxTemp;
	int tempAddress;
	switch(current_mode) {
		case OFFSET:
			minTemp = kMinOffset;
			maxTemp = kMaxOffset;
			tempAddress = OFFSET_ADDRESS;
			break;
		case STEAM:
			minTemp = kMinSteamTemp;
			maxTemp = kMaxSteamTemp;
			tempAddress = STEAM_TEMP_ADDRESS;
			break;
		default:
			minTemp = kMinBrewTemp;
			maxTemp = kMaxBrewTemp;
			tempAddress = BREW_TEMP_ADDRESS;
	}

	if (temp < minTemp) temp = minTemp;
	else if (temp > maxTemp) temp = maxTemp;

	switch(current_mode) {
		case OFFSET:
			this->temp_offset = temp;
			break;
		case STEAM:
			this->target_steam_temp = temp;
			break;
		default:
			this->target_temp = temp;
	}

	EEPROM.put(tempAddress, temp);
	EEPROM.commit();
}

void GCP::incrementTemp(String current_mode) {
	incrementTemp(modeToEnum(current_mode));
}

void GCP::incrementTemp(TempMode current_mode) {
	float temp;
	float i = 0.5;

	if(outputQueue.scale == F) i = (5.0/9.0);
	else if (current_mode == STEAM) i = 1;

	switch(current_mode) {
		case OFFSET: temp = temp_offset;
			break;
		case STEAM: temp = target_steam_temp;
			break;
		default: temp = target_temp;
	}

	temp += i;
	this->setTargetTemp(current_mode, temp);
}

void GCP::decrementTemp(String current_mode) {
	decrementTemp(modeToEnum(current_mode));
}

void GCP::decrementTemp(TempMode current_mode) {
	float temp;
	float i = 0.5;

	if(outputQueue.scale == F) i = (5.0/9.0);
	else if (current_mode == STEAM) i = 1;

	switch(current_mode) {
		case OFFSET: temp = temp_offset;
			break;
		case STEAM: temp = target_steam_temp;
			break;
		default: temp = target_temp;
	}

	temp -= i;
	this->setTargetTemp(current_mode, temp);
}

float GCP::getTargetTemp(TempMode current_mode) {
	switch(current_mode) {
		case OFFSET: 
			return this->temp_offset;
		case STEAM:
			return this->target_steam_temp;
			default: return this->target_temp;
	}
}

float GCP::getActualTemp() {
	float temp =  tempProbe.temperature(100, RREF);
 	uint8_t probe_fault = tempProbe.readFault();
	if (probe_fault) {
		Serial.print("Fault 0x"); Serial.println(probe_fault, HEX);
		if (probe_fault & MAX31865_FAULT_HIGHTHRESH) Serial.println("RTD High Threshold");
		if (probe_fault & MAX31865_FAULT_LOWTHRESH) Serial.println("RTD Low Threshold"); 
		if (probe_fault & MAX31865_FAULT_REFINLOW) Serial.println("REFIN- > 0.85 x Bias");
		if (probe_fault & MAX31865_FAULT_REFINHIGH) Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
		if (probe_fault & MAX31865_FAULT_RTDINLOW) Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
		if (probe_fault & MAX31865_FAULT_OVUV) Serial.println("Under/Over voltage");
		tempProbe.clearFault();
	}
	return temp;
}

float GCP::getCurrentTemp() {
	float temp = this->getActualTemp();
	temp += temp_offset;
	this->current_temp = temp;
	return temp;
}

String GCP::getOutput(){
	return outputQueue.toJson();
}

const char* GCP::getScale(){
	return outputQueue.scale == F ? "F" : "C";
}

String GCP::getTunings(){
	StaticJsonDocument<64> output;
	output["kp"] = kp;
	output["ki"] = ki;
	output["kd"] = kd;
	String outputString;
	serializeJson(output, outputString);
	return outputString;
}

void GCP::setTunings(float kp, float ki, float kd){
	this->kp = kp;
	this->ki = ki;
	this->kd = kd;

	brewTempManager.SetTunings(kp, ki, kd);
	steamTempManager.SetTunings(kp, ki, kd);

	brewTempManager.SetMode(brewTempManager.Control::automatic);
	steamTempManager.SetMode(steamTempManager.Control::automatic);
	
	EEPROM.put(TUNING_ADDRESS, kp);
	EEPROM.put(TUNING_ADDRESS + 8, ki);
	EEPROM.put(TUNING_ADDRESS + 16, kd);
	EEPROM.commit();
}

void GCP::changeScale(String scale) {
	TempScale new_scale = scale == "F" ? F : C;
	outputQueue.scale = new_scale;
	EEPROM.put(SCALE_ADDRESS, new_scale);
	EEPROM.commit();
}

TempMode GCP::modeToEnum(String mode) {
	if(mode == "offset") return OFFSET;
	else if(mode == "steam") return STEAM;
	else return BREW;
}

void GCP::loadParameters(){
	float brew_temp, steam_temp, offset;
	TempScale scale;
	EEPROM.get(BREW_TEMP_ADDRESS, brew_temp);
	EEPROM.get(STEAM_TEMP_ADDRESS, steam_temp);
	EEPROM.get(OFFSET_ADDRESS, offset);
	EEPROM.get(SCALE_ADDRESS, scale);

	if(!isnan(brew_temp) && brew_temp >= kMinBrewTemp && brew_temp <= kMaxBrewTemp) target_temp = brew_temp;
	if(!isnan(steam_temp) && steam_temp >= kMinSteamTemp && steam_temp <= kMaxSteamTemp) target_steam_temp = steam_temp;
	if(!isnan(offset) && offset >= kMinOffset && offset <= kMaxOffset) temp_offset = offset;
	if(scale) outputQueue.scale = scale;

	float tunings[3];
	bool tuningsValid = true;
	for(int i=0; i<3; i++) {
		EEPROM.get(TUNING_ADDRESS + i*8, tunings[i]);
		if(tunings[i] < 0 || isnan(tunings[i])) {
			tuningsValid = false;
			break;
		}
	}
	if(tuningsValid) setTunings(tunings[0], tunings[1], tunings[2]);
}

void GCP::refresh(ulong real_time) {
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

	// Emergency and error handling function
	float actual_temp = this->getActualTemp();
	if(actual_temp >= kEmergencyShutoffTemp || actual_temp < 0) {
		digitalWrite(HEATER_PIN, LOW);
		digitalWrite(STEAM_PIN, LOW);
		return;
	}

	//Update on/off time when the window time is up
	ulong now = millis();
	ulong window_time_elapsed = now - window_start_time;
	if(window_time_elapsed > kWindowSize) {
		brewTempManager.Compute();
		steamTempManager.Compute();
		window_start_time += kWindowSize;
	}

	//Physical hardware controls for brew and steam output
	if(brew_output > window_time_elapsed) digitalWrite(HEATER_PIN, HIGH);
	else digitalWrite(HEATER_PIN, LOW);

	if(steam_output > window_time_elapsed) digitalWrite(STEAM_PIN, HIGH);
	else digitalWrite(STEAM_PIN, LOW);

	//Log information for website display
	ulong log_time_elapsed = now - log_start_time;
	if(log_time_elapsed > kLogInterval) {
		outputQueue.push(
			real_time, 
			this->getCurrentTemp(), 
			this->brew_output, 
			this->steam_output,
			this->target_temp,
			this->target_steam_temp,
			this->temp_offset
		);
		log_start_time += kLogInterval;
	}
}