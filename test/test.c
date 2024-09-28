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

#define TASK_1_PRIORITY      ( tskIDLE_PRIORITY - 1UL )
#define TASK_1_STACK_SIZE configMINIMAL_STACK_SIZE

#define TASK_2_PRIORITY      ( tskIDLE_PRIORITY + 5UL )



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




void thread1(void *params)
{
    struct threadData* td;
    td = (struct threadData*) params;
    
	while (1) {
        vTaskDelay(100);
        printf("beginning of thread1 loop \n");

        if(xSemaphoreTake(td->semaphore1, 10) == pdTRUE){
            td->count1 = td->count1 + 1;
            printf("thread 1 new count1: %d\n", td->count1);

            vTaskDelay(100);

            if(xSemaphoreTake(td->semaphore2, 10000) == pdTRUE){
                td->count2 = td->count2 + 1;
                printf("thread 1 new count2: %d\n", td->count2);
                xSemaphoreGive(td->semaphore2);
            }
            xSemaphoreGive(td->semaphore1);
        }
	}
}

void thread2(void *params)
{
	struct threadData* td;
    td = (struct threadData*) params;
	while (1) {
        vTaskDelay(100);
        printf("beginning of thread2 loop \n");

        if(xSemaphoreTake(td->semaphore2, 10) == pdTRUE){
            td->count2 = td->count2 + 1;
            printf("thread 2 new count2: %d\n", td->count2);




            if(xSemaphoreTake(td->semaphore1, 10000) == pdTRUE){
                td->count2 = td->count2 + 1;
                printf("thread 2 new count1: %d\n", td->count1);
                xSemaphoreGive(td->semaphore1);
            }
            xSemaphoreGive(td->semaphore2);
        }
	}
}







void test_deadlock1(void)
{
    //create semaphores and count variable to pass to threads
    struct threadData td;
    td.semaphore1 = xSemaphoreCreateCounting(1, 1);
    td.semaphore2 = xSemaphoreCreateCounting(1, 1);
    td.count1 = 4;
    td.count2 = 2;

    printf("creating threads\n");

    TaskHandle_t t1, t2, t3;
    BaseType_t x1 = xTaskCreate(thread1, "Thread1",
                TASK_1_STACK_SIZE, (void *) &td, TASK_1_PRIORITY, &t1);
    BaseType_t x2 = xTaskCreate(thread2, "Thread2",
                TASK_1_STACK_SIZE, (void *) &td, TASK_1_PRIORITY, &t2);
    vTaskDelay(1000);
    vTaskDelete(t1);
    vTaskDelete(t2);

    printf("starting tests\n");
    int s1count = uxSemaphoreGetCount(td.semaphore1);
    int s2count = uxSemaphoreGetCount(td.semaphore2);
    printf("starting tests 2\n");
    //TEST_ASSERT_TRUE_MESSAGE(1 == 0,"fail");
    TEST_ASSERT_EQUAL(uxSemaphoreGetCount(td.semaphore1), 0);
    TEST_ASSERT_EQUAL(uxSemaphoreGetCount(td.semaphore1), 0);
    printf("starting tests 3\n");
    TEST_ASSERT_EQUAL(td.count1, 5);
    TEST_ASSERT_EQUAL(td.count2, 3);
    //printf("finished tests\n");
}


void testThread(void *params){

    printf("Start tests\n");
    UNITY_BEGIN();
    
    RUN_TEST(test_loop1);
    RUN_TEST(test_loop2);
    RUN_TEST(test_deadlock1);
    UNITY_END();
    
}


int main (void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    sleep_ms(5000); // Give time for TTY to attach.


    BaseType_t x3 = xTaskCreate(testThread, "testThread",
                configMINIMAL_STACK_SIZE, NULL, TASK_2_PRIORITY, NULL);
    vTaskStartScheduler();

    return 0;
}