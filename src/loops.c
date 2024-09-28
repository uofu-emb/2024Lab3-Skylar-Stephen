#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>

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



//struct for data that gets passed to threads
struct threadData {
  int count1, count2;
  SemaphoreHandle_t semaphore1, semaphore2;
};




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




void deadlock1() {

    //stdio_init_all();
    //hard_assert(cyw43_arch_init() == PICO_OK);
    sleep_ms(5000);
    printf("starting\n");



    //create semaphores and count variable to pass to threads
    struct threadData td;
    td.semaphore1 = xSemaphoreCreateCounting(1, 1);
    td.semaphore2 = xSemaphoreCreateCounting(1, 1);
    td.count1 = 4;
    td.count2 = 2;

    
    TaskHandle_t t1, t2;
    xTaskCreate(thread1, "Thread1",
                TASK_1_STACK_SIZE, (void *) &td, TASK_2_PRIORITY, &t1);
    xTaskCreate(thread2, "Thread2",
                TASK_2_STACK_SIZE, (void *) &td, TASK_2_PRIORITY, &t2);


    vTaskStartScheduler();
	return 0;


}