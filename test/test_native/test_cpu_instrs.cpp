#include <Arduino.h>
#include <SD.h>
#include <unity.h>

SDClass SD;
StdioSerial Serial;

void test_dummy(void) { TEST_ASSERT_EQUAL(15, 0xF); }

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_dummy);
    UNITY_END();

    return 0;
}