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
#include "loops.h"

#define TASK_1_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define TASK_1_STACK_SIZE configMINIMAL_STACK_SIZE

#define TASK_2_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define TASK_2_STACK_SIZE configMINIMAL_STACK_SIZE


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
    
    sleep_ms(5000);
    printf("starting\n");



    //create semaphores and count variable to pass to threads
    struct threadData td;
    td.semaphore1 = xSemaphoreCreateCounting(1, 1);
    td.semaphore2 = xSemaphoreCreateCounting(1, 1);
    td.count1 = 4;
    td.count2 = 2;

    printf("creating threads\n");

    TaskHandle_t t1, t2;
    BaseType_t x1 = xTaskCreate(thread1, "Thread1",
                TASK_1_STACK_SIZE, (void *) &td, TASK_2_PRIORITY, &t1);
    BaseType_t x2 = xTaskCreate(thread2, "Thread2",
                TASK_2_STACK_SIZE, (void *) &td, TASK_2_PRIORITY, &t2);

    printf("done creating threads\n");

    //vTaskStartScheduler();
    vTaskDelay(1000);
    printf("delay finished\n");
    //vTaskSuspend(t1);
    //vTaskSuspend(t2);
    printf("running tests\n");
    TEST_ASSERT_TRUE_MESSAGE(uxSemaphoreGetCount(td.semaphore1) == 0,"fail");
    TEST_ASSERT_TRUE_MESSAGE(uxSemaphoreGetCount(td.semaphore2) == 0,"fail");
    TEST_ASSERT_TRUE_MESSAGE(uxSemaphoreGetCount(td.count1) == 5,"fail");
    TEST_ASSERT_TRUE_MESSAGE(uxSemaphoreGetCount(td.count2) == 3,"fail");

	return 0;

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