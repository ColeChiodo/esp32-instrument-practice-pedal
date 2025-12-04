#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define DEBOUNCE_MS 10

class Button {
	public:
	    Button(gpio_num_t gpio) : gpio_pin(gpio), last_state(1), last_time(0) {
	        gpio_config_t io_conf{};
	        io_conf.intr_type = GPIO_INTR_DISABLE;      // polling
	        io_conf.mode = GPIO_MODE_INPUT;
	        io_conf.pin_bit_mask = (1ULL << gpio_pin);
	        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;    // active-low button
	        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	        gpio_config(&io_conf);
	    }
	
	    // Returns true if the button was just pressed
	    bool isPressed() {
	        int level = gpio_get_level(gpio_pin);  // 0 if pressed
	        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
	
	        if (level == 0 && last_state == 1 && (now - last_time) > DEBOUNCE_MS) {
	            last_time = now;
	            last_state = level;
	            return true;
	        }
	
	        last_state = level;
	        return false;
	    }
	
	private:
	    gpio_num_t gpio_pin;
	    int last_state;
	    uint32_t last_time;
};
