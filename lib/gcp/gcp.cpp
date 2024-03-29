#include "gcp.h"

GCP::GCP()
: tempProbe(Adafruit_MAX31865(THERMOPROBE_PIN))
, kEmergencyShutoffTemp(161.0)
, kMaxBrewTemp(105.0)
, kMinBrewTemp(85.0)
, kMaxSteamTemp(150.0)
, kMinSteamTemp(130.0)
, kMaxOffset(11)
, kMinOffset(-11)
, kWindowSize(1000)
, kLogInterval(500)
, kOutputStep(100)
, kTuneTime(60)
, kSamples(200)
, kSettleTime(5)
, temp_offset(-8)
, target_brew_temp(92)
, target_steam_temp(140)
, brew_output(0)
, steam_output(0)
, brew_kp(40)
, brew_ki(6.67)
, brew_kd(52.27)
, steam_kp(40)
, steam_ki(6.67)
, steam_kd(52.27)
, brewTuner(&current_temp, &brew_output, brewTuner.ZN_PID, brewTuner.directIP, brewTuner.printALL)
, steamTuner(&current_temp, &steam_output, steamTuner.ZN_PID, steamTuner.directIP, steamTuner.printALL)
, brewTempManager(QuickPID(&current_temp, &brew_output, &target_brew_temp))
, steamTempManager(QuickPID(&current_temp, &steam_output, &target_steam_temp))
, outputQueue(Queue(60000 / kLogInterval, C))
{}

GCP::~GCP(){
	digitalWrite(HEATER_PIN, LOW);
	digitalWrite(STEAM_PIN, LOW);
}

void GCP::start() {
	loadParameters();

	if(!tempProbe.begin(MAX31865_3WIRE)) {
		Serial.println("Could not initialize thermocouple.");
		while (1) delay(10);
	}

	pinMode(THERMOPROBE_READY_PIN, INPUT);
	pinMode(HEATER_PIN, OUTPUT);
	pinMode(STEAM_PIN, OUTPUT);

	brewTuner.Configure(kMaxBrewTemp, kWindowSize, 0, kOutputStep, kTuneTime, kSettleTime, kSamples);
	steamTuner.Configure(kMaxSteamTemp, kWindowSize, 0, kOutputStep, kTuneTime, kSettleTime, kSamples);
	brewTuner.SetEmergencyStop(kEmergencyShutoffTemp + temp_offset);
	steamTuner.SetEmergencyStop(kEmergencyShutoffTemp + temp_offset);
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
			brewTuner.SetEmergencyStop(kEmergencyShutoffTemp - temp_offset);
			steamTuner.SetEmergencyStop(kEmergencyShutoffTemp - temp_offset);
			break;
		case STEAM:
			this->target_steam_temp = temp;
			break;
		default:
			this->target_brew_temp = temp;
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
		default: temp = target_brew_temp;
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
		default: temp = target_brew_temp;
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
			default: return this->target_brew_temp;
	}
}

float GCP::getActualTemp() {
	float temp = tempProbe.temperature(100, RREF);
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
	return temp;
}

String GCP::getOutput(){
	return outputQueue.getAll();
}

String GCP::getLastOutput(){
	return outputQueue.getLast();
}

const char* GCP::getScale(){
	return outputQueue.scale == F ? "F" : "C";
}

String GCP::getTunings(String mode){
	return getTunings(modeToEnum(mode));
}

String GCP::getTunings(TempMode mode){
	StaticJsonDocument<48> output;
	if(mode == STEAM) {
		output["kp"] = steam_kp;
		output["ki"] = steam_ki;
		output["kd"] = steam_kd;
	}
	else {
		output["kp"] = brew_kp;
		output["ki"] = brew_ki;
		output["kd"] = brew_kd;
	}

	String outputString;
	serializeJson(output, outputString);
	return outputString;
}

void GCP::setTunings(String mode, float kp, float ki, float kd){
	setTunings(modeToEnum(mode), kp, ki, kd);
}

void GCP::setTunings(TempMode mode, float kp, float ki, float kd){
	QuickPID* tempManager;
	int eeprom_offset = 0;
	
	if(mode == STEAM) {
		steam_kp = kp;
		steam_ki = ki;
		steam_kd = kd;
		tempManager = &steamTempManager;
		eeprom_offset = 24;
	}
	else {
		brew_kp = kp;
		brew_ki = ki;
		brew_kd = kd;
		tempManager = &brewTempManager;
	}

	tempManager->SetTunings(kp, ki, kd);

	EEPROM.put(TUNING_ADDRESS + eeprom_offset, kp);
	EEPROM.put(TUNING_ADDRESS + eeprom_offset + 8, ki);
	EEPROM.put(TUNING_ADDRESS + eeprom_offset + 16, kd);
	EEPROM.commit();
}

void GCP::changeScale(String scale_string) {
	TempScale scale = scale_string == "F" ? F : C;
	outputQueue.scale = scale;
	EEPROM.put(SCALE_ADDRESS, scale);
	EEPROM.commit();
}

TempMode GCP::modeToEnum(String mode) {
	if(mode == "offset") return OFFSET;
	else if(mode == "steam") return STEAM;
	else return BREW;
}

float GCP::PWM(TempMode mode, QuickPID* tempManager, sTune* tuner, int pin, float output, float setpoint){
	ulong now = millis();
	if(output > (now - window_start_time)) digitalWrite(pin, HIGH);
	else digitalWrite(pin, LOW);

	if((tuning_complete || current_temp > (target_steam_temp * 0.9)) && !currently_pwm) currently_pwm = true;

	switch(tuner->Run()){
		case tuner->sample:
			current_temp = this->getCurrentTemp();
			if(!currently_pwm) steam_output = kWindowSize;
			break;
		case tuner->tunings:
			float kp, ki, kd;
			tuner->GetAutoTunings(&kp, &ki, &kd);
			kp *= 10;
			ki *= 10;
			kd *= 10;
			if(mode == STEAM) {
				steam_output = kOutputStep;
				steam_kp = kp;
				steam_ki = ki;
				steam_kd = kd;
			}
			else {
				brew_output = kOutputStep;
				brew_kp = kp;
				brew_ki = ki;
				brew_kd = kd;
			}
			tempManager->SetOutputLimits(0, kWindowSize);
			tempManager->SetSampleTimeUs((kWindowSize - 1) * 1000);
			tempManager->SetMode(tempManager->Control::automatic);
			tempManager->SetProportionalMode(tempManager->pMode::pOnMeas);
			tempManager->SetAntiWindupMode(tempManager->iAwMode::iAwClamp);
			setTunings(mode, kp, ki, kd);
			tuning_complete = true;
			break;
		case tuner->runPid:
			current_temp = this->getCurrentTemp();
			tempManager->Compute();
			if(current_temp < setpoint * 0.9) {
				if(mode == STEAM) steam_output = kWindowSize;
				else brew_output = kWindowSize;
			}
			else if(current_temp < setpoint * 0.975) {
				if(mode == STEAM) steam_output = kWindowSize / 2;
				else brew_output = kWindowSize / 2;
			}
			break;
	}
	return output;
}

void GCP::loadParameters(){
	float brew_temp, steam_temp, offset;
	TempScale scale;
	EEPROM.get(BREW_TEMP_ADDRESS, brew_temp);
	EEPROM.get(STEAM_TEMP_ADDRESS, steam_temp);
	EEPROM.get(OFFSET_ADDRESS, offset);
	EEPROM.get(SCALE_ADDRESS, scale);

	if(!isnan(brew_temp) && brew_temp >= kMinBrewTemp && brew_temp <= kMaxBrewTemp) target_brew_temp = brew_temp;
	if(!isnan(steam_temp) && steam_temp >= kMinSteamTemp && steam_temp <= kMaxSteamTemp) target_steam_temp = steam_temp;
	if(!isnan(offset) && offset >= kMinOffset && offset <= kMaxOffset) temp_offset = offset;
	if(scale) outputQueue.scale = scale;

	float tunings[6];
	bool tuningsValid = true;
	for(int i=0; i<6; i++) {
		EEPROM.get(TUNING_ADDRESS + i*8, tunings[i]);
		if(tunings[i] < 0 || isnan(tunings[i])) {
			tuningsValid = false;
			break;
		}
	}
	if(tuningsValid) {
		setTunings(BREW, tunings[0], tunings[1], tunings[2]);
		setTunings(STEAM, tunings[3], tunings[4], tunings[5]);
	}
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

	if(this->getActualTemp() > kEmergencyShutoffTemp) {
		digitalWrite(HEATER_PIN, LOW);
		digitalWrite(STEAM_PIN, LOW);
		return;
	}

	ulong now = millis();
	ulong window_elapsed_time = now - window_start_time;
	if(window_elapsed_time > kWindowSize) {
		window_start_time += kWindowSize;
	}

	float optimum_brew = PWM(BREW, &brewTempManager, &brewTuner, HEATER_PIN, brew_output, target_brew_temp);
	float optimum_steam = PWM(STEAM, &steamTempManager, &steamTuner, STEAM_PIN, steam_output, target_steam_temp);

	//Log information for website display
	ulong log_time_elapsed = now - log_start_time;
	if(log_time_elapsed > kLogInterval) {
		outputQueue.push(
			real_time, 
			current_temp, 
			optimum_brew, 
			optimum_steam,
			this->target_brew_temp,
			this->target_steam_temp,
			this->temp_offset
		);
		log_start_time += kLogInterval;
	}
}