#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "unity_config.h"

void setUp(void) {}

void tearDown(void) {}

void test_loop1()
{
    int count = 0;
    SemaphoreHandle_t semaphore;
    semaphore = xSemaphoreCreateCounting(1, 1);
    
    xSemaphoreTake(semaphore, 5000);

    loop1(semaphore, &count, 0);
    
    //TEST_ASSERT_TRUE_MESSAGE(xSemaphoreTake(semaphore, 0) == pdFALSE,"fail");
    TEST_ASSERT_TRUE_MESSAGE(count == 0,"fail");

    xSemaphoreGive(semaphore);

    loop1(semaphore, &count, 6000);
    TEST_ASSERT_TRUE_MESSAGE(count == 1,"fail");
}

void test_loop2(void)
{
    int x = 30;
    int y = 6;
    int z = x / y;
    TEST_ASSERT_TRUE_MESSAGE(z == 5, "Multiplication of two integers returned incorrect value.");
}

int main (void)
{
    stdio_init_all();
    sleep_ms(5000); // Give time for TTY to attach.
    printf("Start tests\n");
    UNITY_BEGIN();
    RUN_TEST(test_loop1);
    RUN_TEST(test_loop2);
    sleep_ms(5000);
    return UNITY_END();
}
