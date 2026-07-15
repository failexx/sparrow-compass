/*
* A big thanks to the guys below:
* Rui Santos & Sara Santos - Random Nerd Tutorials
* https://RandomNerdTutorials.com/esp-idf-esp32-gpio-interrupts/
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include "sdkconfig.h"

#define BUTTON_MID_GPIO GPIO_NUM_15       // Pushbutton GPIO
#define BUTTON_UP_GPIO GPIO_NUM_17
#define BUTTON_DOWN_GPIO GPIO_NUM_16
#define BUTTON_LEFT_GPIO GPIO_NUM_4
#define BUTTON_RIGHT_GPIO GPIO_NUM_18

volatile int* button_ptr;

#define DEBOUNCE_DELAY_US 200000ULL  // Debounce delay in microseconds (200 ms)

static volatile uint64_t last_isr_time = 0;
static volatile uint32_t counter = 0;
static QueueHandle_t button_queue;


// Interrupt Service Routine (ISR) for button press, placed in IRAM for low latency
static void IRAM_ATTR button_isr(void *arg) {
    uint64_t now = esp_timer_get_time(); // Get current time in microseconds
    // Check if debounce period has passed, then process the button press
    if (now - last_isr_time > DEBOUNCE_DELAY_US) {
        button_ptr = (int*)arg;
        counter++;
        uint32_t cnt = counter;
        BaseType_t higher_priority_task_woken = pdFALSE;
        xQueueSendFromISR(button_queue, &cnt, &higher_priority_task_woken); // Send counter to queue from ISR
        last_isr_time = now;
        if (higher_priority_task_woken) {
            portYIELD_FROM_ISR();
        }
    }
}

void button_main(void) {
    printf("Press a button");

    // Create a queue to hold up to 10 uint32_t items
    button_queue = xQueueCreate(10, sizeof(uint32_t));

    // Configure Mid Button GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE, // Rising edge interrupt trigger
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_MID_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf);

    // Configure Up Button GPIO
    gpio_config_t io_conf2 = {
        .intr_type = GPIO_INTR_POSEDGE, // Rising edge interrupt trigger
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_UP_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf2);

    // Configure Down Button GPIO
    gpio_config_t io_conf3 = {
        .intr_type = GPIO_INTR_POSEDGE, // Rising edge interrupt trigger
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_DOWN_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf3);

    // Configure Left Button GPIO
    gpio_config_t io_conf4 = {
        .intr_type = GPIO_INTR_POSEDGE, // Rising edge interrupt trigger
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_LEFT_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf4);

    // Configure Right Button GPIO
    gpio_config_t io_conf5 = {
        .intr_type = GPIO_INTR_POSEDGE, // Rising edge interrupt trigger
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_RIGHT_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf5);

    // Install GPIO ISR service
    gpio_install_isr_service(0);

    // Add ISR handler for button
    gpio_isr_handler_add(BUTTON_MID_GPIO, button_isr, (void*)BUTTON_MID_GPIO);
    gpio_isr_handler_add(BUTTON_UP_GPIO, button_isr, (void*)BUTTON_UP_GPIO);
    gpio_isr_handler_add(BUTTON_DOWN_GPIO, button_isr, (void*)BUTTON_DOWN_GPIO);
    gpio_isr_handler_add(BUTTON_LEFT_GPIO, button_isr, (void*)BUTTON_LEFT_GPIO);
    gpio_isr_handler_add(BUTTON_RIGHT_GPIO, button_isr, (void*)BUTTON_RIGHT_GPIO);

    // Variable to receive counter from queue
    uint32_t button_counter;

    // Keep program running
    while (1) {
        // Wait indefinitely for an item in the queue
        if (xQueueReceive(button_queue, &button_counter, portMAX_DELAY)) {
            printf("Knappen sa klick %lu times.\n", button_counter);
            if ((int)button_ptr == BUTTON_MID_GPIO) {
                printf("Mid button pressed.\n");
                button_ptr = NULL;
            } else if ((int)button_ptr == BUTTON_UP_GPIO) {
                printf("Up button pressed.\n");
                button_ptr = NULL;
            } else if ((int)button_ptr == BUTTON_DOWN_GPIO) {
                printf("Down button pressed.\n");
                button_ptr = NULL;
            } else if ((int)button_ptr == BUTTON_LEFT_GPIO) {
                printf("Left button pressed.\n");
                button_ptr = NULL;
            } else if ((int)button_ptr == BUTTON_RIGHT_GPIO) {
                printf("Right button pressed.\n");
                button_ptr = NULL;
                //CALL ON OLED FUNCTION TO CHANGE DISPLAY
            }
        }
    }
}