#include "header.h"

WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
OLED screen;
GCP brew_machine;
const char* hostname = "gaggia";

void setup() {
	while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
	//Primary Mission: start coffee machine
	Serial.begin(115200);

	WiFi.mode(WIFI_STA);
	WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
	  while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
	}
	Serial.print("IP Address: ");
	Serial.print(WiFi.localIP());
	Serial.println("");

	if (!MDNS.begin(hostname)) { //http://gaggia.local
		Serial.println("Error setting up MDNS responder!");
	}
	Serial.println("Running on http://gaggia.local");

	server.on("/", HTTP_GET, []() {
		server.send(200, "text/html", indexHtml);
	});

	server.on("/get_temps", HTTP_GET, [](){
		server.send(200, "text/json", brew_machine.getOutput());
	});

	server.on("/get_tunings/brew", HTTP_GET, [](){
		server.send(200, "text/json", brew_machine.getTunings("brew"));
	});

	server.on("/get_tunings/steam", HTTP_GET, [](){
		server.send(200, "text/json", brew_machine.getTunings("steam"));
	});

	server.on("/set_tunings", HTTP_PUT, [](){
		double tunings[3];
		String params = server.arg(0);
		String mode = "";
		int stringIndex = params.indexOf(",");
		int arrayIndex = -1;
		while(stringIndex > 0) {
			String substr = params.substring(0, stringIndex);
			params = params.substring(stringIndex + 1);
			stringIndex = params.indexOf(",");
			if(arrayIndex == -1) {
				mode = substr;
			}
			else {
				double result = substr.toDouble();
				tunings[arrayIndex] = result;
			}
			arrayIndex++;
		}
		tunings[arrayIndex] = params.toDouble();
		brew_machine.setTunings(mode, tunings[0], tunings[1], tunings[2]);
		server.send(200, "text/plain", "Success!");
	});

	server.on("/autotune/brew", HTTP_GET, []() {
		brew_machine.autoTune("brew", &server);
	});

	server.on("/autotune/steam", HTTP_GET, []() {
		brew_machine.autoTune("steam", &server);
		server.send(200, "text/plain", "Success!");
	});

	server.on("/autotune/brew/cancel", HTTP_GET, []() {
		brew_machine.cancelAutoTune("brew");
		server.send(200, "text/plain", "Success!");
	});

	server.on("/autotune/steam/cancel", HTTP_GET, []() {
		brew_machine.cancelAutoTune("steam");
		server.send(200, "text/plain", "Success!");
	});

	server.on("/increment_target", HTTP_POST, [](){
		brew_machine.incrementTemp(server.arg(0));
		server.send(200, "text/plain", "Success!");
	});

	server.on("/decrement_target", HTTP_POST, [](){
		brew_machine.decrementTemp(server.arg(0));
		server.send(200, "text/plain", "Success!");
	});

	server.on("/set_offset", HTTP_POST, [](){
		String data = server.arg(0);
		double offset = data.toDouble();
		brew_machine.setTempOffset(offset);
		server.send(200, "text/plain", "Success!");
	});

	server.onNotFound([]() {
		server.send(404, "text/plain", "Not Found");
	});

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
	server.begin();
	timeClient.begin();

	EEPROM.begin(512);
	brew_machine.start();
	screen.start(&brew_machine);
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	server.handleClient();
	timeClient.update();

	ulong currentTime = timeClient.getEpochTime();
	if(currentTime) {
		brew_machine.refresh(timeClient.getEpochTime());
		screen.refresh();
	}
}