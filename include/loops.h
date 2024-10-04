#pragma once
#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>

void loop1(SemaphoreHandle_t semaphore, int *counter, int ticks);
void loop2(SemaphoreHandle_t semaphore, int *counter, int *on);
void thread1(void *params);
void thread2(void *params);

struct threadData {
  int count1, count2;
  SemaphoreHandle_t semaphore1, semaphore2;
};