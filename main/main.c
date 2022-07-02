#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LoRa/lora.h"
#include "Ultrasonic/ultrasonic.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ECHO_GPIO 12
#define TRIGGER_GPIO 13
#define MAX_DISTANCE_CM 600 // Maximum of 5 meters

float distance;
uint8_t send_distance[4];



void ultrasonic_test(void *pvParameters)
{

    ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIGGER_GPIO,
        .echo_pin = ECHO_GPIO
    };

    ultrasonic_init(&sensor);

    while (true) {
        esp_err_t res = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);

        if (res == ESP_OK) {
            // printf("Distance: %0.04f m\n", distance);
        } // Print error
        else {
            printf("Error %d: ", res);
            switch (res) {
                case ESP_ERR_ULTRASONIC_PING:
                    printf("Cannot ping (device is in invalid state)\n");
                    break;
                case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                    printf("Ping timeout (no device found)\n");
                    break;
                case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                    printf("Echo timeout (i.e. distance too big)\n");
                    break;
                default:
                     // continue;
                    printf("%s\n", esp_err_to_name(res));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void task_tx(void *p)
{
   lora_init();
   lora_set_frequency(915e6);
   lora_enable_crc();
   void float_uint8_t(float *val, uint8_t data[]){
      memcpy(data, val, sizeof(float));
   }
   
   for(;;) {
      float_uint8_t(&distance,send_distance);
      vTaskDelay(pdMS_TO_TICKS(1000));
      lora_send_packet((uint8_t*)send_distance, 4);
      printf("packet sent: %f...\n", distance);
   }
}

void app_main()
{
   
   xTaskCreate(&task_tx, "task_tx", 2048, NULL, 5, NULL);
   xTaskCreate(ultrasonic_test, "ultrasonic_test", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);

}