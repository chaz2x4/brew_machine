#include <header.h>
#include <unity.h>
#include <ArduinoJson.h> 

GCP brew_machine;
OLED screen;
double saved_target_brew_temp;
double saved_target_steam_temp;
double saved_target_offset;
double saved_tunings[3];
ulong timer_start_time;

int current_loop = 0;
int max_loops = 30;

void loadParameters(){
    EEPROM.get(BREW_TEMP_ADDRESS, saved_target_brew_temp);
    EEPROM.get(STEAM_TEMP_ADDRESS, saved_target_steam_temp);
    EEPROM.get(OFFSET_ADDRESS, saved_target_offset);
    for(int i=0;i<3;i++) {
        EEPROM.get(TUNING_ADDRESS + i*8, saved_tunings[i]);
    }
}

void saveParameters(){
    brew_machine.setTargetTemp(BREW, saved_target_brew_temp);
    brew_machine.setTargetTemp(STEAM, saved_target_steam_temp);
    brew_machine.setTargetTemp(OFFSET, saved_target_offset);
    brew_machine.setTunings(saved_tunings[0], saved_tunings[1], saved_tunings[2]);
}

void test_function_gcp_set_brew_temp(){
    brew_machine.setTargetTemp(BREW, 95);
    TEST_ASSERT_EQUAL(95, brew_machine.getTargetTemp(BREW));
}

void test_function_gcp_set_steam_temp(){
    brew_machine.setTargetTemp(STEAM, 150);
    TEST_ASSERT_EQUAL(150, brew_machine.getTargetTemp(STEAM));
}

void test_function_gcp_set_offset(){
     brew_machine.setTargetTemp(OFFSET, -10);
    TEST_ASSERT_EQUAL(-10, brew_machine.getTargetTemp(OFFSET));
}

void test_function_gcp_increment_brew_temp(){
    brew_machine.setTargetTemp(BREW, 95);
    brew_machine.incrementTemp(BREW);
    TEST_ASSERT_EQUAL(95.5, brew_machine.getTargetTemp(BREW));
}

void test_function_gcp_increment_steam_temp(){
    brew_machine.setTargetTemp(STEAM, 149);
    brew_machine.incrementTemp(STEAM);
    TEST_ASSERT_EQUAL(150, brew_machine.getTargetTemp(STEAM));
}

void test_function_gcp_increment_offset(){
    brew_machine.setTargetTemp(OFFSET, -10);
    brew_machine.incrementTemp(OFFSET);
    TEST_ASSERT_EQUAL(-9.5, brew_machine.getTargetTemp(OFFSET));
}

void test_function_gcp_decrement_brew_temp(){
    brew_machine.setTargetTemp(BREW, 95);
    brew_machine.decrementTemp(BREW);
    TEST_ASSERT_EQUAL(94.5, brew_machine.getTargetTemp(BREW));
}

void test_function_gcp_decrement_steam_temp(){
    brew_machine.setTargetTemp(STEAM, 150);
    brew_machine.decrementTemp(STEAM);
    TEST_ASSERT_EQUAL(149, brew_machine.getTargetTemp(STEAM));
}

void test_function_gcp_decrement_offset(){
    brew_machine.setTargetTemp(OFFSET, 10);
    brew_machine.decrementTemp(OFFSET);
    TEST_ASSERT_EQUAL(9.5, brew_machine.getTargetTemp(OFFSET));
}

void test_function_gcp_brew_max_1(){
    brew_machine.setTargetTemp(BREW, 162);
    TEST_ASSERT_EQUAL(105, brew_machine.getTargetTemp(BREW));
}

void test_function_gcp_brew_max_2(){
    brew_machine.setTargetTemp(BREW, 105);
    brew_machine.incrementTemp(BREW);
    TEST_ASSERT_EQUAL(105, brew_machine.getTargetTemp(BREW));
}

void test_function_gcp_brew_min_1(){
    brew_machine.setTargetTemp(BREW, 0);
    TEST_ASSERT_EQUAL(85, brew_machine.getTargetTemp(BREW));
}

void test_function_gcp_brew_min_2(){
    brew_machine.setTargetTemp(BREW, 0);
    brew_machine.decrementTemp(BREW);
    TEST_ASSERT_EQUAL(85, brew_machine.getTargetTemp(BREW));
}

void test_function_gcp_steam_max_1(){
    brew_machine.setTargetTemp(STEAM, 162);
    TEST_ASSERT_EQUAL(150, brew_machine.getTargetTemp(STEAM));
}

void test_function_gcp_steam_max_2(){
    brew_machine.setTargetTemp(STEAM, 150);
    brew_machine.incrementTemp(STEAM);
    TEST_ASSERT_EQUAL(150, brew_machine.getTargetTemp(STEAM));
}

void test_function_gcp_steam_min_1(){
    brew_machine.setTargetTemp(STEAM, 0);
    TEST_ASSERT_EQUAL(130, brew_machine.getTargetTemp(STEAM));
}

void test_function_gcp_steam_min_2(){
    brew_machine.setTargetTemp(STEAM, 130);
    brew_machine.decrementTemp(STEAM);
    TEST_ASSERT_EQUAL(130, brew_machine.getTargetTemp(STEAM));
}

void test_function_gcp_offset_max_1(){
    brew_machine.setTargetTemp(OFFSET, 20);
    TEST_ASSERT_EQUAL(11, brew_machine.getTargetTemp(OFFSET));
}

void test_function_gcp_offset_min_1(){
    brew_machine.setTargetTemp(OFFSET, -20);
    TEST_ASSERT_EQUAL(-11, brew_machine.getTargetTemp(OFFSET));
}

void test_function_gcp_offset_max_2(){
    brew_machine.setTargetTemp(OFFSET, 20);
    brew_machine.incrementTemp(OFFSET);
    TEST_ASSERT_EQUAL(15, brew_machine.getTargetTemp(OFFSET));
}

void test_function_gcp_offset_min_2(){
    brew_machine.setTargetTemp(OFFSET, -20);
    brew_machine.decrementTemp(OFFSET);
    TEST_ASSERT_EQUAL(-15, brew_machine.getTargetTemp(OFFSET));
}

void test_function_gcp_current_temp(){
    double temp = brew_machine.getActualTemp();
    double offset = brew_machine.getTargetTemp(OFFSET);
    TEST_ASSERT_EQUAL(temp + offset, brew_machine.getCurrentTemp());
}

void test_function_gcp_set_tunings(){
    brew_machine.setTunings(10, 10, 10);
    TEST_ASSERT_EQUAL_STRING("{ \"kp\": 10.00, \"ki\": 10.00, \"kd\": 10.00 }", brew_machine.getTunings().c_str());
}

void test_function_gcp_eeprom_brew(){
    double temp;
    brew_machine.setTargetTemp(BREW, 96);
    EEPROM.get(BREW_TEMP_ADDRESS, temp);
    TEST_ASSERT_EQUAL(96, temp);
}

void test_function_gcp_eeprom_steam(){
    double temp;
    brew_machine.setTargetTemp(STEAM, 152);
    EEPROM.get(STEAM_TEMP_ADDRESS, temp);
    TEST_ASSERT_EQUAL(152, temp);
}

void test_function_gcp_eeprom_offset(){
    double temp;
    brew_machine.setTargetTemp(OFFSET, -11);
    EEPROM.get(OFFSET_ADDRESS, temp);
    TEST_ASSERT_EQUAL(-11, temp);
}

void test_function_gcp_eeprom_tunings(){
    double actualTunings[3];
    double expectedTunings[3] = {11, 12, 13};
    brew_machine.setTunings(11, 12, 13);
    for(int i=0; i<3 ; i++) {
        EEPROM.get(TUNING_ADDRESS + i*8, actualTunings[i]);
        TEST_ASSERT_EQUAL(expectedTunings[i], actualTunings[i]);
    }
}

void test_function_oled_increment_brew(){
    TempMode current_mode = screen.getCurrentMode();
    double target = brew_machine.getTargetTemp(BREW);
    if(current_mode == BREW) {
        screen.incrementTemp();
        TEST_ASSERT_EQUAL(target + 0.5, brew_machine.getTargetTemp(BREW));
    }
    else {
        screen.changeMode();
        test_function_oled_increment_brew();
    }
}

void test_function_oled_decrement_brew(){
    TempMode current_mode = screen.getCurrentMode();
    double target = brew_machine.getTargetTemp(BREW);
    if(current_mode == BREW) {
        screen.decrementTemp();
        TEST_ASSERT_EQUAL(target - 0.5, brew_machine.getTargetTemp(BREW));
    }
    else {
        screen.changeMode();
        test_function_oled_decrement_brew();
    }
}

void test_function_oled_increment_steam(){
    TempMode current_mode = screen.getCurrentMode();
    double target = brew_machine.getTargetTemp(STEAM);
    if(current_mode == STEAM) {
        screen.incrementTemp();
        TEST_ASSERT_EQUAL(target + 1, brew_machine.getTargetTemp(STEAM));
    }
    else {
        screen.changeMode();
        test_function_oled_increment_steam();
    }
}

void test_function_oled_decrement_steam(){
    TempMode current_mode = screen.getCurrentMode();
    double target = brew_machine.getTargetTemp(STEAM);
    if(current_mode == STEAM) {
        screen.decrementTemp();
        TEST_ASSERT_EQUAL(target - 1, brew_machine.getTargetTemp(STEAM));
    }
    else {
        screen.changeMode();
        test_function_oled_decrement_steam();
    }
}

void test_function_gcp_output(){
    ulong now = millis();
    brew_machine.refresh(now);
}

void setup(){
    delay(2000);
    brew_machine.start();
    screen.start(&brew_machine);
    EEPROM.begin(512);
    loadParameters();    

    UNITY_BEGIN();
    RUN_TEST(test_function_gcp_set_brew_temp);
    RUN_TEST(test_function_gcp_set_steam_temp);
    RUN_TEST(test_function_gcp_set_offset);
    RUN_TEST(test_function_gcp_increment_brew_temp);
    RUN_TEST(test_function_gcp_decrement_brew_temp);
    RUN_TEST(test_function_gcp_increment_steam_temp);
    RUN_TEST(test_function_gcp_decrement_steam_temp);
    RUN_TEST(test_function_gcp_increment_offset);
    RUN_TEST(test_function_gcp_decrement_offset);
    RUN_TEST(test_function_gcp_brew_max_1);
    RUN_TEST(test_function_gcp_brew_min_1);
    RUN_TEST(test_function_gcp_steam_max_1);
    RUN_TEST(test_function_gcp_steam_min_1);
    RUN_TEST(test_function_gcp_brew_max_2);
    RUN_TEST(test_function_gcp_brew_min_2);
    RUN_TEST(test_function_gcp_steam_max_2);
    RUN_TEST(test_function_gcp_steam_min_2);
    RUN_TEST(test_function_gcp_offset_max_1);
    RUN_TEST(test_function_gcp_offset_min_1);
    RUN_TEST(test_function_gcp_offset_max_2);
    RUN_TEST(test_function_gcp_offset_min_2);
    RUN_TEST(test_function_gcp_current_temp);
    RUN_TEST(test_function_gcp_set_tunings);
    RUN_TEST(test_function_gcp_eeprom_brew);
    RUN_TEST(test_function_gcp_eeprom_steam);
    RUN_TEST(test_function_gcp_eeprom_offset);
    RUN_TEST(test_function_gcp_eeprom_tunings);

    RUN_TEST(test_function_oled_increment_brew);
    RUN_TEST(test_function_oled_increment_steam);
    // RUN_TEST(test_function_oled_increment_offset);
    RUN_TEST(test_function_oled_decrement_brew);
    RUN_TEST(test_function_oled_decrement_steam);
    // RUN_TEST(test_function_oled_decrement_offset);

    timer_start_time = millis();
}

void loop(){
    if(current_loop < max_loops) {
        RUN_TEST(test_function_gcp_output);
        delay(1000);
        current_loop++;
    }
    else {
        saveParameters();
        digitalWrite(HEATER_PIN, LOW);
		digitalWrite(STEAM_PIN, LOW);
        UNITY_END();
    }
}