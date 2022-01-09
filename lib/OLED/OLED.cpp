#include "OLED.h"

OLED::OLED()
: display(Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire))
, currentMode("brew")
, timeLastButton(-1)
, lastTime(-1)
, downTime(-1)
, isEditable(false)
, flash(false)
, buttonState{HIGH, HIGH, HIGH}
, lastButtonState{HIGH, HIGH, HIGH}
, screenTimeout(900000)
, triggerTime(1000)
{}

OLED::~OLED(){
    delete gcp;
}

void OLED::start(GCP* brew_machine){
    display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
    display.clearDisplay();
    display.dim(true);
    display.display();

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);
    timeLastButton = millis();

    gcp = brew_machine;
}

void OLED::eventListener(){
    buttonState[0] = digitalRead(BUTTON_A);
    buttonState[1] = digitalRead(BUTTON_B);
    buttonState[2] = digitalRead(BUTTON_C);

    ulong now = millis();

    if(checkedIfTimedout()) { //on any button press immediately turn screen on, but dont do anything else
        if((buttonState[0] == LOW && lastButtonState[0] == HIGH) ||
            (buttonState[1] == LOW && lastButtonState[1] == HIGH) ||
            (buttonState[2] == LOW && lastButtonState[2] == HIGH)) {
                timeLastButton = now;
                lastButtonState[0] = buttonState[0];
                lastButtonState[1] = buttonState[1];
                lastButtonState[2] = buttonState[2];
        }
    }
    else {
        //Increase temperature when editable on button A
        if(buttonState[0] == LOW && lastButtonState[0] == HIGH) {
            downTime = now;
            timeLastButton = now;
            if(this->isEditable) this->incrementTemp();
            else this->changeMode();
        }
        else if(buttonState[0] == HIGH && lastButtonState[0] == LOW) downTime = -1 ;
        lastButtonState[0] = buttonState[0];

        //Decrease temperature when editable on button B
        if(buttonState[1] == LOW && lastButtonState[1] == HIGH) {
            downTime = now;
            timeLastButton = now;
            if(this->isEditable) this->decrementTemp();
            else this->changeMode();
        }
        else if(buttonState[1] == HIGH && lastButtonState[1] == LOW) downTime = -1 ;
        lastButtonState[1] = buttonState[1];

        //Change mode if button C is pressed
        if(buttonState[2] == LOW && lastButtonState[2] == HIGH) {
            downTime = now;
            timeLastButton = now;
            if(this->isEditable) {
                this->isEditable = false;
                lastTime = now;
                flash = true;
            }
        }
        else if(buttonState[2] == HIGH && lastButtonState[2] == LOW) downTime = -1; 
        if(buttonState[2] == LOW && (now - downTime) >= triggerTime) this->isEditable = true;
        lastButtonState[2] = buttonState[2];
        display.clearDisplay();
    }
}

bool OLED::checkedIfTimedout(){
    ulong now = millis();
    if((now - timeLastButton) >= screenTimeout) {
        this->isEditable = false;
        display.clearDisplay();
        display.display();
        return true;
    }
    return false;
}

void OLED::refresh(ulong realTime){
    this->realTime = realTime;
    this->eventListener();
    if(checkedIfTimedout()) return;
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13, 0);
    display.clearDisplay();

    ulong wait;
    ulong now = millis();
    char* modeTitle = "Brew";
    double targetTemp = gcp->getTargetTemp("brew");
    double currentTemp = gcp->getCurrentTemp();
    if(this->currentMode == "steam") {
        modeTitle = "Steam";
        targetTemp = gcp->getTargetTemp("steam");
    }
    if(this->isEditable) {
        if(flash) display.printf("Set %s\n %#.1f C", modeTitle, targetTemp);
        else display.printf("Set %s", modeTitle);
        wait = 500;
    }
    else {
        if(flash) display.printf("%sing\n %#0.1f C", modeTitle, targetTemp);
        else display.printf("Temp:\n %#.1f C", currentTemp);
        wait = 2000;
    }

    if((now - lastTime) > wait) {
        lastTime = now;
        flash = !flash;
    }
    display.display();
}

void OLED::changeMode(){
    display.clearDisplay();
    lastTime = millis();
    flash = true;
    this->currentMode = this->currentMode == "brew" ? "steam" : "brew";
}

String OLED::getCurrentMode(){
    return this->currentMode;;
}

void OLED::incrementTemp(){
    gcp->incrementTemp(this->currentMode);
}

void OLED::decrementTemp(){
    gcp->decrementTemp(this->currentMode);
}