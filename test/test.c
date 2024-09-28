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
    //Take semaphore, then run code and make sure count was not incremented 
    xSemaphoreTake(semaphore, 0);
    loop1(semaphore, &count, 10);
    TEST_ASSERT_TRUE_MESSAGE(count == 0,"fail");

    //Return semaphore, run code and verify it ran
    xSemaphoreGive(semaphore);
    loop1(semaphore, &count, 0);
    TEST_ASSERT_TRUE_MESSAGE(count == 1,"fail");
}

void test_loop2(void)
{
    hard_assert(cyw43_arch_init() == PICO_OK);
    int count = 0;
    int on = 0;
    SemaphoreHandle_t semaphore;
    semaphore = xSemaphoreCreateCounting(1, 1);
    //Take semaphore, then run code and make sure count was not incremented and on was not changed
    xSemaphoreTake(semaphore, 0);
    loop2(semaphore, &count, &on);
    TEST_ASSERT_TRUE_MESSAGE(count == 0,"fail");
    TEST_ASSERT_TRUE_MESSAGE(on == 0,"fail");

    //Return semaphore, run code and verify it ran
    xSemaphoreGive(semaphore);
    loop2(semaphore, &count, &on);
    TEST_ASSERT_TRUE_MESSAGE(count == 1,"fail");
    TEST_ASSERT_TRUE_MESSAGE(on == 1,"fail");

}

void test_deadlock1(void)
{
    deadlock1();
    sleep_ms(3000);

}

int main (void)
{
    stdio_init_all();
    sleep_ms(5000); // Give time for TTY to attach.
    printf("Start tests\n");
    UNITY_BEGIN();
    RUN_TEST(test_loop1);
    RUN_TEST(test_loop2);
    RUN_TEST(test_deadlock1);
    sleep_ms(5000);
    return UNITY_END();
}