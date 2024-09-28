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
    if(xSemaphoreTake(semaphore, 1000) == pdTRUE){
            *counter = *counter + 1;
            printf("hello world from %s! Count %d\n", "main", *counter);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, *on);
            *on = !(*on);
            xSemaphoreGive(semaphore);
        }
}




