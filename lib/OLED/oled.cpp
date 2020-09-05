// 
// 
// 

#include "oled.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

void OLED::start(){
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.display();
    delay(1000);

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);
}

ulong downTime = -1;
int triggerTime = 1000;
bool buttonAState = HIGH;
bool buttonBState = HIGH;
bool buttonCState = HIGH;
bool lastButtonState = HIGH;
void OLED::eventListener(){
    //Change mode if button C is pressed
    buttonCState = digitalRead(BUTTON_C);
    if(buttonCState == LOW && lastButtonState == HIGH) {
        downTime = millis();
        if(this->isEditable) {
            this->isEditable = false;
        }
    }
    else if(buttonCState == HIGH && lastButtonState == LOW) {
        downTime = -1;
    }
    if(buttonCState == LOW && this->currentMode == brew && (millis() - downTime) >= triggerTime) {
        this->isEditable = true;
    }
    lastButtonState = buttonCState;
}

ulong lastTime = -1;
bool flash = false;
void wait (int delay, char* text1, float var1, char* text2, float var2){
    if((millis() - lastTime) >= delay && flash) {
        display.clearDisplay();
        display.printf(text1, var1);
        lastTime = millis();
        flash = !flash;
    } 
    else if((millis() - lastTime) >= delay && !flash) {
        display.clearDisplay();
        display.printf(text2, var2);
        lastTime = millis();
        flash = !flash;
    }
}

void OLED::refresh(){
    this->update();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13, 0);
    if(this->currentMode == brew) {
        if(this->isEditable) {
            wait(500,(char*)("Set Brew: %.1f C"), this->getTargetTemp(), (char*)("Set Brew: "), 0x0);
        }
        else {
            wait(2000, (char*)("Pressure: %.1f bars\n"), this->getPX(), (char*)("Temp: \n %.1f C\n"), this->getActualTemp());
        }
    }
    if(this->currentMode == steam) {
        display.println("Steam");
    }
    display.display();
}

void OLED::changeMode(){
    if(this->currentMode == brew) this->currentMode = steam;
    else this->currentMode = brew;
}