#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "loops.h"

#define TASK_1_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define TASK_1_STACK_SIZE configMINIMAL_STACK_SIZE

#define TASK_2_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define TASK_2_STACK_SIZE configMINIMAL_STACK_SIZE





void loop1(SemaphoreHandle_t semaphore, int *counter, int ticks) {
    if(xSemaphoreTake(semaphore, ticks) == pdTRUE){
            *counter = *counter + 1;
            printf("hello world from %s! Count %d\n", "thread", *counter);
            xSemaphoreGive(semaphore);
        }
}

void loop2(SemaphoreHandle_t semaphore, int *counter, int *on) {
    if(xSemaphoreTake(semaphore, 100000) == pdTRUE){
            *counter = *counter + 1;
            printf("hello world from %s! Count %d\n", "main", *counter);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, *on);
            *on = !(*on);
            xSemaphoreGive(semaphore);
        }
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