/* gpio example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"

static const char *TAG = "main";

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO15: output
 * GPIO16: output
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Test:
 * Connect GPIO15 with GPIO4
 * Connect GPIO16 with GPIO5
 * Generate pulses on GPIO15/16, that triggers interrupt on GPIO4/5
 *
 */


// Changed pin number below to 2
#define GPIO_OUTPUT_IO_0    2
//#define GPIO_OUTPUT_IO_1    16
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO_0)
// Changed pin number below to 0
#define GPIO_INPUT_IO_0     0
//#define GPIO_INPUT_IO_1     5
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)

//static xQueueHandle gpio_evt_queue = NULL;

SemaphoreHandle_t xSemaphore = NULL;

void vApplicationIdleHook(void){
	//configure esp to wake up from light sleep after 10 ms
	esp_sleep_enable_timer_wakeup(10000);
	// start light sleep
	esp_light_sleep_start();
}


static void turn_gpio_on(void *arg){
while(1){
	int count1 = 0;
	TickType_t xTime1, xTime2, xExecutionTime;
	xTime1 = 0;
	xTime2 = 0;
	xExecutionTime = 0;
		if (xSemaphoreTake (xSemaphore, portMAX_DELAY) == pdTRUE){
			gpio_set_level(GPIO_OUTPUT_IO_0, 1);
			xTime1 = xTaskGetTickCount();
			// After 50 ticks / 500ms has occurred, exit while loop
				while(xExecutionTime < 50){
					count1++;
					xTime2 = xTaskGetTickCount();
					xExecutionTime = xTime2 - xTime1;
				}
				vTaskDelay(1000 / portTICK_RATE_MS);
				xSemaphoreGive(xSemaphore);			 	 
			}
		vTaskDelay( pdMS_TO_TICKS(10) );
	}
}

static void turn_gpio_off(void *arg){
while(1){
	int count2 = 0;
	TickType_t xTime1, xTime2, xExecutionTime;
	xTime1 = 0;
	xTime2 = 0;
	xExecutionTime = 0;
		if (xSemaphoreTake (xSemaphore, portMAX_DELAY) == pdTRUE){
			gpio_set_level(GPIO_OUTPUT_IO_0, 0);
			xTime1 = xTaskGetTickCount();
			// After 50 ticks / 500ms has occurred, exit while loop
			while(xExecutionTime < 50){
				count2++;
				xTime2 = xTaskGetTickCount();
				xExecutionTime = xTime2 - xTime1;
			}
			vTaskDelay(1000 / portTICK_RATE_MS);
			xSemaphoreGive(xSemaphore);
        }
	 vTaskDelay( pdMS_TO_TICKS(10) );
	}
}

static void status_message(void *arg){
	while(1){
		ESP_LOGI(TAG, "GPIO[2] Level: %d", gpio_get_level(2));
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}


void app_main(void)
{
	gpio_config_t io_conf;
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set,e.g.GPIO15/16
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
	
	// Create mutex
	xSemaphore = xSemaphoreCreateMutex();
	// Creates all tasks
	xTaskCreate(turn_gpio_on, "turn_gpio_on", 2048, NULL, 8, NULL); // The priority of each task will be changed for each test case
	xTaskCreate(turn_gpio_off, "turn_gpio_off", 2048, NULL, 10, NULL);	
	xTaskCreate(status_message, "status_message", 2048, NULL, 9, NULL);

	
	for(;;){
		vTaskDelay( pdMS_TO_TICKS(10) );
	}
}

