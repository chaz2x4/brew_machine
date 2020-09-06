// 
// 
// 

#include "oled.h"

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
int buttonState[3] = {HIGH, HIGH, HIGH};
int lastButtonState[3] = {HIGH, HIGH, HIGH};
void OLED::eventListener(){
    buttonState[0] = digitalRead(BUTTON_A);
    buttonState[1] = digitalRead(BUTTON_B);
    buttonState[2] = digitalRead(BUTTON_C);

    //Increase temperature when editable on button A
    if(buttonState[0] == LOW && lastButtonState[0] == HIGH) {
        downTime = millis();
        if(this->isEditable) this->incrementTemp();
        else this->changeMode();
    }
    else if(buttonState[0] == HIGH && lastButtonState[0] == LOW) { downTime = -1 ; }
    lastButtonState[0] = buttonState[0];

    //Decrease temperature when editable on button B
    if(buttonState[1] == LOW && lastButtonState[1] == HIGH) {
        downTime = millis();
        if(this->isEditable) this->decrementTemp();
        else this->changeMode();
    }
    else if(buttonState[1] == HIGH && lastButtonState[1] == LOW) { downTime = -1 ; }
    lastButtonState[1] = buttonState[1];

    //Change mode if button C is pressed
    if(buttonState[2] == LOW && lastButtonState[2] == HIGH) {
        downTime = millis();
        if(this->isEditable) {
            this->isEditable = false;
        }
    }
    else if(buttonState[2] == HIGH && lastButtonState[2] == LOW) {
        downTime = -1;
    }
    if(buttonState[2] == LOW && this->currentMode == brew && (millis() - downTime) >= triggerTime) {
        this->isEditable = true;
    }
    lastButtonState[2] = buttonState[2];
}

ulong lastTime = -1;
bool flash = false;
void OLED::wait (int delay, char* text1, float var1, char* text2, float var2){
    if((millis() - lastTime) >= delay) {
        display.clearDisplay();
        if(flash) display.printf(text1, var1);
        else display.printf(text2, var2);
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
            wait(400, (char *)("Set Brew: %.1f C"), this->getTargetTemp(), (char *)("Set Brew: "), 0x0);
        }
        else {
            wait(1900, (char *)("Pressure: %.1f bar\n"), this->getPX(), (char *)("Temp: \n %.1f C\n"), this->getActualTemp());
        }
    }
    if(this->currentMode == steam) {
        wait(900, (char *)("Steaming. %.1f C\n"), this->getActualTemp(), (char *)("Steaming  %.1f C\n"), this->getActualTemp());
    }
    display.display();
}

void OLED::changeMode(){
    display.clearDisplay();
    if(this->currentMode == brew) this->currentMode = steam;
    else this->currentMode = brew;
}