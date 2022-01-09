#include <GCP.h>
#include <unity.h>

GCP brew_machine;

void setUp(){

}

void test_function_gcp_set_brew_temp(){
    brew_machine.setTargetTemp("brew", 95);
    TEST_ASSERT_EQUAL(95, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_set_steam_temp(){
    brew_machine.setTargetTemp("steam", 150);
    TEST_ASSERT_EQUAL(150, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_set_offset(){
    brew_machine.setTempOffset(-10);
    TEST_ASSERT_EQUAL(-10, brew_machine.getTempOffset());
}

void test_function_gcp_increment_brew_temp(){
    brew_machine.setTargetTemp("brew", 95);
    brew_machine.incrementTemp("brew");
    TEST_ASSERT_EQUAL(95.5, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_increment_steam_temp(){
    brew_machine.setTargetTemp("steam", 150);
    brew_machine.incrementTemp("steam");
    TEST_ASSERT_EQUAL(151, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_decrement_brew_temp(){
    brew_machine.setTargetTemp("brew", 95);
    brew_machine.decrementTemp("brew");
    TEST_ASSERT_EQUAL(94.5, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_decrement_steam_temp(){
    brew_machine.setTargetTemp("steam", 150);
    brew_machine.decrementTemp("steam");
    TEST_ASSERT_EQUAL(149, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_brew_max(){
    brew_machine.setTargetTemp("brew", 162);
    TEST_ASSERT_EQUAL(115, brew_machine.getTargetTemp("brew"));
    brew_machine.incrementTemp("brew");
    TEST_ASSERT_EQUAL(115, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_brew_min(){
    brew_machine.setTargetTemp("brew", 0);
    TEST_ASSERT_EQUAL(85, brew_machine.getTargetTemp("brew"));
    brew_machine.decrementTemp("brew");
    TEST_ASSERT_EQUAL(85, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_steam_max(){
    brew_machine.setTargetTemp("steam", 162);
    TEST_ASSERT_EQUAL(160, brew_machine.getTargetTemp("steam"));
    brew_machine.incrementTemp("steam");
    TEST_ASSERT_EQUAL(160, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_steam_min(){
    brew_machine.setTargetTemp("steam", 0);
    TEST_ASSERT_EQUAL(140, brew_machine.getTargetTemp("steam"));
    brew_machine.decrementTemp("steam");
    TEST_ASSERT_EQUAL(140, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_current_temp(){
    double temp = brew_machine.getActualTemp();
    double offset = brew_machine.getTempOffset();
    TEST_ASSERT_EQUAL(temp + offset, brew_machine.getCurrentTemp());
}

void test_function_gcp_set_tunings(){
    brew_machine.setTunings(10, 10, 10);
    TEST_ASSERT_EQUAL_STRING("{ \"kp\": 10.00, \"ki\": 10.00, \"kd\": 10.00 }", brew_machine.getTunings().c_str());
}

void setup(){
    delay(2000);
    brew_machine.start();
    
    UNITY_BEGIN();
    RUN_TEST(test_function_gcp_set_brew_temp);
    RUN_TEST(test_function_gcp_set_steam_temp);
    RUN_TEST(test_function_gcp_set_offset);
    RUN_TEST(test_function_gcp_increment_brew_temp);
    RUN_TEST(test_function_gcp_decrement_brew_temp);
    RUN_TEST(test_function_gcp_increment_steam_temp);
    RUN_TEST(test_function_gcp_decrement_steam_temp);
    RUN_TEST(test_function_gcp_brew_max);
    RUN_TEST(test_function_gcp_brew_min);
    RUN_TEST(test_function_gcp_steam_max);
    RUN_TEST(test_function_gcp_steam_min);
    RUN_TEST(test_function_gcp_current_temp);
    RUN_TEST(test_function_gcp_set_tunings);
    UNITY_END();
}


void loop(){

}