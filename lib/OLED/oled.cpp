// 
// 
// 

#include "oled.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

void OLED::start(){
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    delay(1000);

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);
}

void OLED::buttonTest() {

}

void OLED::clear(){
    display.clearDisplay();
}

void OLED::event(){

}

void OLED::refresh(){

}

void OLED::changeMode(){

}