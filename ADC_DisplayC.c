#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define VRX_PIN 26  // Pino do eixo X do joystick
#define VRY_PIN 27  // Pino do eixo Y do joystick
#define LED_RED_PIN 13  // Pino do LED Vermelho
#define LED_BLUE_PIN 12 // Pino do LED Azul
#define LED_GREEN_PIN 11 // Pino do LED Verde
#define BUTTON_JOYSTICK 22 // Botão do joystick
#define BUTTON_A 5 // Botão A

bool led_green_state = false;
bool pwm_enabled = true;

uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

uint16_t process_joystick_value(uint16_t value) {
    if (value >= 2048 + 200 || value <= 2048 - 200) { // Adicionando zona morta
        if (value >= 2048) {
            return (value - 2048) * 2;
        } else {
            return (2048 - value) * 2;
        }
    }
    return 0; // Mantém o LED apagado na posição central
}

void toggle_led_green() {
    led_green_state = !led_green_state;
    gpio_put(LED_GREEN_PIN, led_green_state);
}

void toggle_pwm_state() {
    pwm_enabled = !pwm_enabled;
    pwm_set_enabled(pwm_gpio_to_slice_num(LED_RED_PIN), pwm_enabled);
    pwm_set_enabled(pwm_gpio_to_slice_num(LED_BLUE_PIN), pwm_enabled);
}

int main() {
    stdio_init_all();
    adc_init(); 
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);
    
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    
    uint pwm_wrap = 4095;
    uint pwm_red_slice = pwm_init_gpio(LED_RED_PIN, pwm_wrap);
    uint pwm_blue_slice = pwm_init_gpio(LED_BLUE_PIN, pwm_wrap);

    uint32_t last_print_time = 0;
    bool last_button_joystick = true;
    bool last_button_a = true;

    while (true) {
        adc_select_input(0);  
        uint16_t vrx_value = adc_read(); 
        adc_select_input(1);  
        uint16_t vry_value = adc_read(); 
        
        uint16_t red_brightness = process_joystick_value(vrx_value);
        uint16_t blue_brightness = process_joystick_value(vry_value);
        
        if (pwm_enabled) {
            pwm_set_gpio_level(LED_RED_PIN, red_brightness);
            pwm_set_gpio_level(LED_BLUE_PIN, blue_brightness);
        }
        
        bool current_button_joystick = gpio_get(BUTTON_JOYSTICK);
        if (!current_button_joystick && last_button_joystick) {
            toggle_led_green();
        }
        last_button_joystick = current_button_joystick;
        
        bool current_button_a = gpio_get(BUTTON_A);
        if (!current_button_a && last_button_a) {
            toggle_pwm_state();
        }
        last_button_a = current_button_a;

        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_print_time >= 1000) {
            printf("VRX: %u, Red Brightness: %u\n", vrx_value, red_brightness);
            printf("VRY: %u, Blue Brightness: %u\n", vry_value, blue_brightness);
            last_print_time = current_time;
        }

        sleep_ms(100);
    }

    return 0;
}