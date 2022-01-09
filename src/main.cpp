#include "header.h"

AsyncWebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
OLED screen;
GCP brew_machine;
const char* hostname = "gaggia";

void setup() {
	while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
	//Primary Mission: start coffee machine
	Serial.begin(115200);

	EEPROM.begin(512);
	brew_machine.start();
	screen.start(&brew_machine);

	WiFi.mode(WIFI_STA);
	WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {}

	ArduinoOTA.setHostname(hostname);
	ArduinoOTA.onStart([](){
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
		else type = "filesystem";
		Serial.println("Updating " + type);
	}).onEnd([](){
		Serial.println("\nComplete");
	}).onProgress([](uint progress, uint total){
		Serial.printf("Progress %u%%\r", (progress / (total / 1000)));
	}).onError([](ota_error_t error){
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.begin();

	if (!MDNS.begin(hostname)) { //http://gaggia.local
		Serial.println("Error setting up MDNS responder!");
	}
	Serial.println("Running on http://gaggia.local");

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send_P(200, "text/html", indexHtml);
	});

	server.on("/get_temps", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "text/json", brew_machine.getOutput());
	});

	server.on("/get_tunings", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "text/json", brew_machine.getTunings());
	});

	AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/set_tunings", [](AsyncWebServerRequest *request, JsonVariant &json) {
		JsonObject&& tunings = json.as<JsonObject>();
		double kp = tunings["kp"].as<double>();
		double ki = tunings["ki"].as<double>();
		double kd = tunings["kd"].as<double>();
		if(kp && ki && kd) {
			brew_machine.setTunings(kp, ki, kd);
			request->send(200);
		}
		else request->send(400);
	});
	server.addHandler(handler);

	server.on("/increment_target", HTTP_POST, [](AsyncWebServerRequest *request){
		if(request->hasArg("mode")) { 
			brew_machine.incrementTemp(request->arg("mode"));
			request->send(200);
		}
		else request->send(400);
	});

	server.on("/decrement_target", HTTP_POST, [](AsyncWebServerRequest *request){
		if(request->hasArg("mode")) { 
			brew_machine.decrementTemp(request->arg("mode"));
			request->send(200);
		}
		else request->send(400);
	});

	server.onNotFound([](AsyncWebServerRequest *request) {
		request->send(404);
	});

	server.begin();
	timeClient.begin();
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	timeClient.update();

	ulong currentTime = timeClient.getEpochTime();
	if(currentTime) {
		brew_machine.refresh(currentTime);
		screen.refresh(currentTime);
	}
}