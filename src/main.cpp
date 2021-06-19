#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#include "header.h"
#include "passwords.h"

WebServer server(80);
OLED brew_machine;

void setup() {
	while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
	//Primary Mission: start coffee machine
	Serial.begin(115200);

	WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
	  while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.printf("IP Address: %s\n", WiFi.localIP());
	if (!MDNS.begin("gaggia")) { //http://gaggia.local
		Serial.println("Error setting up MDNS responder!");
		while (1) {
		delay(1000);
		}
	}

	server.on("set_tunings", HTTP_POST, [](){
		server.sendHeader("Connection", "close");
		server.send(200, "text/plain", (Update.hasError()) ? "FAILED TO UPDATE TUNINGS" : "UPDATED TUNINGS");
	}, [](){
		Serial.println("Updating...");
	});

	/*handling uploading firmware file */
	server.on("/update_firmware", HTTP_POST, []() {
		server.sendHeader("Connection", "close");
		server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
		ESP.restart();
	}, []() {
		HTTPUpload& upload = server.upload();
		if (upload.status == UPLOAD_FILE_START) {
		Serial.printf("Update: %s\n", upload.filename.c_str());
		if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
			Update.printError(Serial);
		}
		} else if (upload.status == UPLOAD_FILE_WRITE) {
		/* flashing firmware to ESP*/
		if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			Update.printError(Serial);
		}
		} else if (upload.status == UPLOAD_FILE_END) {
		if (Update.end(true)) { //true to set the size to the current progress
			Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
		} else {
			Update.printError(Serial);
		}
		}
	});

	server.begin();
	brew_machine.start();
}

// the loop function runs over and over again until power down or reset
void loop() {
	brew_machine.refresh();
	brew_machine.eventListener();
	yield();
}