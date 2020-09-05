// 
// 
// 

#include "oled.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

void OLED::start(){
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    display.clearDisplay();
    delay(1000);

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);
}

void OLED::event(){

}

bool flash = false;
void OLED::refresh(){
    this->update();
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    if(this->currentMode == brew) {
        if(this->isEditable) {
            display.println(); // targetTemp
            delay(250);
            if(!flash) {
                display.clearDisplay();
                flash = true;
            }
            else {
                display.println(this->getTargetTemp());
                flash = false;
            }
        }
        else {
            display.println("Brew");
        }
    }
    if(this->currentMode == steam) {
        display.println("Steam");
    }
      display.setCursor(0,0);
      display.display();
}

void OLED::changeMode(){
    if(this->currentMode == brew) this->currentMode = steam;
    else this->currentMode = brew;
}