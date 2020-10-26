#include "header.h"
#include "unity.h"

GCP bm;

void test_constructor(){
    GCP bm(92.0);
    TEST_ASSERT_EQUAL_FLOAT(92.0, bm.getTargetTemp());
}

void test_defaultTargetTemp() {
    TEST_ASSERT_EQUAL_FLOAT(DEFAULT_TARGET_TEMP, bm.getTargetTemp());
}

void test_setTargetTemp() {
    bm.setTargetTemp(92.0);
    TEST_ASSERT_EQUAL_FLOAT(92.0, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void test_incrementTemp(){
    float targetTemp = bm.getTargetTemp() + 0.5;
    bm.incrementTemp();
    TEST_ASSERT_EQUAL_FLOAT(targetTemp, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void test_decrementTemp(){
    float targetTemp = bm.getTargetTemp() - 0.5;
    bm.decrementTemp();
    TEST_ASSERT_EQUAL_FLOAT(targetTemp, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void test_maxBrewTemp(){
    bm.setTargetTemp(MAX_BREW_TEMP + 10);
    TEST_ASSERT_EQUAL_FLOAT(MAX_BREW_TEMP, bm.getTargetTemp());
    bm.incrementTemp();
    TEST_ASSERT_EQUAL_FLOAT(MAX_BREW_TEMP, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void test_minBrewTemp(){
    bm.setTargetTemp(MIN_BREW_TEMP - 10);
    TEST_ASSERT_EQUAL_FLOAT(MIN_BREW_TEMP, bm.getTargetTemp());
    bm.decrementTemp();
    TEST_ASSERT_EQUAL_FLOAT(MIN_BREW_TEMP, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_constructor);

    RUN_TEST(test_defaultTargetTemp);
    RUN_TEST(test_setTargetTemp);

    RUN_TEST(test_incrementTemp);
    RUN_TEST(test_decrementTemp);

    RUN_TEST(test_minBrewTemp);
    RUN_TEST(test_maxBrewTemp);
}

void loop () {
    UNITY_END();
}