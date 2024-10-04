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

//This function tests the loop1 function
void test_loop1()
{
    //initialize count variable and semaphore
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

//This function tests the loop2 function
void test_loop2(void)
{
    //initialize count variable and semaphore
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



//thread 1 for deadlock test
void thread1(void *params)
{
    //put input data into new threadData struct
    struct threadData* td;
    td = (struct threadData*) params;
    

	while (1) {
        vTaskDelay(100);
        printf("beginning of thread1 loop \n");

        //take semaphore1 and increment count
        if(xSemaphoreTake(td->semaphore1, 10) == pdTRUE){
            td->count1 = td->count1 + 1;
            printf("thread 1 new count1: %d\n", td->count1);

            vTaskDelay(100);
            
            //attempt to take semaphore2 and increment count
            if(xSemaphoreTake(td->semaphore2, 10000) == pdTRUE){
                td->count2 = td->count2 + 1;
                printf("thread 1 new count2: %d\n", td->count2);
                xSemaphoreGive(td->semaphore2);
            }
            xSemaphoreGive(td->semaphore1);
        }
	}
}

//thread 2 for deadlock test
void thread2(void *params)
{
    //put input data into new threadData struct
	struct threadData* td;
    td = (struct threadData*) params;


	while (1) {
        vTaskDelay(100);
        printf("beginning of thread2 loop \n");

        //take semaphore2 and increment count
        if(xSemaphoreTake(td->semaphore2, 10) == pdTRUE){
            td->count2 = td->count2 + 1;
            printf("thread 2 new count2: %d\n", td->count2);

            //attempt to take semaphore1 and increment count
            if(xSemaphoreTake(td->semaphore1, 10000) == pdTRUE){
                td->count2 = td->count2 + 1;
                printf("thread 2 new count1: %d\n", td->count1);
                xSemaphoreGive(td->semaphore1);
            }
            xSemaphoreGive(td->semaphore2);
        }
	}
}

//This code creates an orphaned lock
void orphaned_lock(void* params)
{
    struct threadData* td;
    td = (struct threadData*) params;

    while (1) {
        xSemaphoreTake(td->semaphore1, 10000);
        td->count1 = td->count1 + 1;
        if (td->count1 % 2) {
            //continue before giving semaphore back
            continue;
        }
        printf("Count %d\n", td->count1);
        xSemaphoreGive(td->semaphore1);
    }
}

//This code has the same functionality as orphaned_lock but does not create the orphaned lock
void orphaned_lock_fixed(void* params)
{
    struct threadData* td;
    td = (struct threadData*) params;

    while (1) {
        xSemaphoreTake(td->semaphore1, 10000);
        td->count1 = td->count1 + 1;
        if (td->count1 % 2) {
            xSemaphoreGive(td->semaphore1);
            continue;
        }
        printf("Count %d\n", td->count1);
        xSemaphoreGive(td->semaphore1);

        //stop after 10000
        if (td->count1 > 10000)
            vTaskDelay(1000);
    }
}



//code that creates deadlock, then tests to make sure deadlock was actually created
void test_deadlock1(void)
{
    //create semaphores and count variable to pass to threads
    struct threadData td;
    td.semaphore1 = xSemaphoreCreateCounting(1, 1);
    td.semaphore2 = xSemaphoreCreateCounting(1, 1);
    td.count1 = 4;
    td.count2 = 2;

    //Create both threads to put them in deadlock
    printf("creating threads\n");
    TaskHandle_t t1, t2, t3;
    BaseType_t x1 = xTaskCreate(thread1, "Thread1",
                TASK_1_STACK_SIZE, (void *) &td, TASK_1_PRIORITY, &t1);
    BaseType_t x2 = xTaskCreate(thread2, "Thread2",
                TASK_1_STACK_SIZE, (void *) &td, TASK_1_PRIORITY, &t2);
    vTaskDelay(1000);
    vTaskDelete(t1);
    vTaskDelete(t2);

    //Test if both semaphores were taken by the threads
    TEST_ASSERT_EQUAL(uxSemaphoreGetCount(td.semaphore1), 0);
    TEST_ASSERT_EQUAL(uxSemaphoreGetCount(td.semaphore1), 0);

    //Test if each count was only incremented once
    TEST_ASSERT_EQUAL(td.count1, 5);
    TEST_ASSERT_EQUAL(td.count2, 3);

}

//This function tests the orphaned_lock and orphan_lock_fixed functions
void test_orphan(void) 
{
    //Create threadData struct to pass count and semaphore to threads
    struct threadData td;
    td.semaphore1 = xSemaphoreCreateCounting(1, 1);
    td.count1 = 0;

    //create orphaned lock thread
    TaskHandle_t t1;
    BaseType_t x1 = xTaskCreate(orphaned_lock, "Thread1",
                TASK_1_STACK_SIZE, (void *) &td, TASK_1_PRIORITY, &t1);
    vTaskDelay(1000);
    vTaskDelete(t1);

    //test semaphore count, if 0 then thread is deadlocked
    printf("test orphan semaphore count\n");
    TEST_ASSERT_EQUAL(uxSemaphoreGetCount(td.semaphore1), 0);
    //make sure count was incremented correctly
    TEST_ASSERT_EQUAL(td.count1, 1);

    //Re initialize semaphore and count
    td.semaphore1 = xSemaphoreCreateCounting(1, 1);
    td.count1 = 0;

    //create orphan lock fixed thread
    TaskHandle_t t2;
    BaseType_t x2 = xTaskCreate(orphaned_lock_fixed, "Thread2",
                TASK_1_STACK_SIZE, (void *) &td, TASK_1_PRIORITY, &t2);
    vTaskDelay(1000);
    vTaskDelete(t2);

    //Make sure semaphore was given back correctly and count was incremented
    TEST_ASSERT_EQUAL(uxSemaphoreGetCount(td.semaphore1), 1);
    TEST_ASSERT_TRUE(td.count1 > 0);


}

//thread that runs all tests
void testThread(void *params){

    printf("Start tests\n");
    UNITY_BEGIN();
    RUN_TEST(test_loop1);
    RUN_TEST(test_loop2);
    RUN_TEST(test_deadlock1);
    RUN_TEST(test_orphan);
    UNITY_END();
    
}


int main (void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    sleep_ms(5000); // Give time for TTY to attach.

    //create test thread to run tests
    BaseType_t x3 = xTaskCreate(testThread, "testThread",
                configMINIMAL_STACK_SIZE, NULL, TASK_2_PRIORITY, NULL);
    vTaskStartScheduler();

    return 0;
}