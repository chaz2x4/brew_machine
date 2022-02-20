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
, kWindowSize(1500)
, kLogInterval(500)
, kPowerFrequency(60)
, temp_offset(-8)
, target_temp(92)
, target_steam_temp(140)
, preinfusion_time(2000)
, temp_kp(127.5)
, temp_ki(15.55)
, temp_kd(392.06)
, brewTempManager(PID(&current_temp, &brew_output, &target_temp, temp_kp, temp_ki, temp_kd, P_ON_M, DIRECT))
, steamTempManager(PID(&current_temp, &steam_output, &target_steam_temp, temp_kp, temp_ki, temp_kd, P_ON_M, DIRECT))
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
	pinMode(TRANSDUCER_PIN, INPUT);

	brewTempManager.SetMode(AUTOMATIC);
	steamTempManager.SetMode(AUTOMATIC);

	brewTempManager.SetOutputLimits(0, kWindowSize);
	steamTempManager.SetOutputLimits(0, kWindowSize);
	brewTempManager.SetSampleTime(kLogInterval);
	steamTempManager.SetSampleTime(kLogInterval);
}

void GCP::setTargetTemp(TempMode current_mode, double temp) {
	double minTemp;
	double maxTemp;
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
	double temp;
	double i = 0.5;

	if(outputQueue.current_scale == F) i = (5.0/9.0);
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
	double temp;
	double i = 0.5;

	if(outputQueue.current_scale == F) i = (5.0/9.0);
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

double GCP::getTargetPressure() {
	return this->target_pressure;
}

double GCP::getTargetTemp(TempMode current_mode) {
	switch(current_mode) {
		case OFFSET: 
			return this->temp_offset;
		case STEAM:
			return this->target_steam_temp;
			default: return this->target_temp;
	}
}

double GCP::getActualTemp() {
	double temp =  tempProbe.temperature(100, RREF);
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

double GCP::getCurrentTemp() {
	double temp = this->getActualTemp();
	temp += temp_offset;
	this->current_temp = temp;
	return temp;
}

String GCP::getOutput(){
	String outputString = "[";

	for(unsigned i = 0; i < outputQueue.size(); i++){
		if(i > 0 ) outputString += ",";
		outputString += outputQueue.at(i);
	}

	outputString += "]";
	return outputString;
}

String GCP::getTunings(){
	String output;
    output += "{ \"kp\": ";
    output += temp_kp;
    output += ", \"ki\": ";
    output += temp_ki;
    output +=  ", \"kd\": ";
    output += temp_kd;
    output += " }";
    return output;
}

void GCP::setTunings(double kp, double ki, double kd){
	this->temp_kp = kp;
	this->temp_ki = ki;
	this->temp_kd = kd;

	brewTempManager.SetTunings(kp, ki, kd, P_ON_M);
	brewTempManager.SetMode(AUTOMATIC);

	steamTempManager.SetTunings(kp, ki, kd, P_ON_M);
	steamTempManager.SetMode(AUTOMATIC);
	
	EEPROM.put(TUNING_ADDRESS, kp);
	EEPROM.put(TUNING_ADDRESS + 8, ki);
	EEPROM.put(TUNING_ADDRESS + 16, kd);
	EEPROM.commit();
}

void GCP::changeScale(String scale) {
	if(outputQueue.getScale() == scale) return;
	if(scale == "F") {
		outputQueue.setScale(F);
	}
	else if(scale == "C") { 
		outputQueue.setScale(C); 
	};
}

TempMode GCP::modeToEnum(String mode) {
	if(mode == "offset") return OFFSET;
	else if(mode == "steam") return STEAM;
	else return BREW;
}

void GCP::loadParameters(){
	double brew_temp, steam_temp, offset;
	EEPROM.get(BREW_TEMP_ADDRESS, brew_temp);
	EEPROM.get(STEAM_TEMP_ADDRESS, steam_temp);
	EEPROM.get(OFFSET_ADDRESS, offset);

	if(!isnan(brew_temp) && brew_temp >= kMinBrewTemp && brew_temp <= kMaxBrewTemp) target_temp = brew_temp;
	if(!isnan(steam_temp) && steam_temp >= kMinSteamTemp && steam_temp <= kMaxSteamTemp) target_steam_temp = steam_temp;
	if(!isnan(offset) && offset >= kMinOffset && offset <= kMaxOffset) temp_offset = offset;

	double tunings[3];
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

int GCP::regulateOutput(double output) {
	int roundedOutput = int(output);
	int powerPeriod = int(1000/kPowerFrequency);
	int remainder = roundedOutput % powerPeriod;
	if(remainder == 0) return roundedOutput;
	return roundedOutput + powerPeriod - remainder;
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

	double actual_temp = this->getActualTemp();
	if(actual_temp >= kEmergencyShutoffTemp || actual_temp < 0) {
		digitalWrite(HEATER_PIN, LOW);
		digitalWrite(STEAM_PIN, LOW);
		return;
	}

	ulong now = millis();
	if(now - log_start_time > kLogInterval) {
		brewTempManager.Compute();
		steamTempManager.Compute();
		outputQueue.push(
			real_time, 
			this->getCurrentTemp(), 
			this->last_brew_output, 
			this->last_steam_output,
			this->target_temp,
			this->target_steam_temp,
			this->temp_offset
		);
		log_start_time += kLogInterval;
	}
	
	if(now - window_start_time > kWindowSize) {
		window_start_time += kWindowSize;
		last_brew_output = regulateOutput(brew_output);
		last_steam_output = regulateOutput(steam_output);

		if(target_steam_temp - current_temp > 5) {
			steamTempManager.SetMode(MANUAL);
			last_steam_output = kWindowSize;
		}
		else if(target_steam_temp - current_temp < -1) { 
			steamTempManager.SetMode(MANUAL);
			last_steam_output = 0;
		}
		else {
			steamTempManager.SetMode(AUTOMATIC);
		}
		
		if(target_temp - current_temp > 5)  {
			brewTempManager.SetMode(MANUAL);
			last_brew_output = kWindowSize;
		}
		else if(target_temp - current_temp < -1) {
			brewTempManager.SetMode(MANUAL);
			last_brew_output = 0;
		} else {
			brewTempManager.SetMode(AUTOMATIC);
		}
	}

	if(last_brew_output > now - window_start_time) digitalWrite(HEATER_PIN, HIGH);
	else digitalWrite(HEATER_PIN, LOW);

	if(last_steam_output > now - window_start_time) digitalWrite(STEAM_PIN, HIGH);
	else digitalWrite(STEAM_PIN, LOW);
	
}