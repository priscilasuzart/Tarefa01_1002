#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define JOYSTICK_X_PIN 26  // GPIO para eixo X
#define JOYSTICK_Y_PIN 27  // GPIO para eixo Y
#define JOYSTICK_PB 22 // GPIO para botão do Joystick
#define Botao_A 5 // GPIO para botão A
#define LED_RED_PIN 13  // Pino do LED Vermelho
#define LED_BLUE_PIN 12 // Pino do LED Azul
#define LED_GREEN_PIN 11 // Pino do LED Verde

// Variáveis globais
bool led_green_state = false;
bool pwm_enabled = true;
uint pwm_wrap = 4095;
bool borda_alternada = false;
bool cor = true;  // Controle da cor da borda
uint32_t last_button_press = 0; // Armazena o tempo do último pressionamento do botão
const uint32_t debounce_time = 200; // Debounce de 200ms para o botão

// Função para inicializar o PWM nos LEDs
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

// Função para processar os valores do joystick com uma zona morta
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

// Função para alternar o estado do LED Verde
void toggle_led_green() {
    led_green_state = !led_green_state;
    gpio_put(LED_GREEN_PIN, led_green_state);
}

// Função para alternar o estado do PWM (ativar/desativar)
void toggle_pwm_state() {
    pwm_enabled = !pwm_enabled;
    pwm_set_enabled(pwm_gpio_to_slice_num(LED_RED_PIN), pwm_enabled);
    pwm_set_enabled(pwm_gpio_to_slice_num(LED_BLUE_PIN), pwm_enabled);
}


int main() {
    stdio_init_all();
    
    // Inicialização de I2C e display SSD1306
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Definindo o pino como I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Definindo o pino como I2C
    gpio_pull_up(I2C_SDA); // Pino de dados
    gpio_pull_up(I2C_SCL); // Pino de clock

    ssd1306_t ssd; 
    ssd1306_init(&ssd, 128, 64, false, endereco, I2C_PORT); 
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicialização dos pinos do joystick e LEDs
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB); 

    gpio_init(Botao_A);
    gpio_set_dir(Botao_A, GPIO_IN);
    gpio_pull_up(Botao_A);

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    
    uint pwm_red_slice = pwm_init_gpio(LED_RED_PIN, pwm_wrap);
    uint pwm_blue_slice = pwm_init_gpio(LED_BLUE_PIN, pwm_wrap);

    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);  

    uint16_t adc_value_x, adc_value_y;
    int x_pos = 60, y_pos = 32; // Posição inicial do quadrado no display

    uint32_t last_print_time = 0;
    uint32_t last_button_joystick = 0;
    uint32_t last_button_a = 0;

    while (true) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());

        // Leitura dos valores do joystick
        adc_select_input(0);  
        adc_value_x = adc_read(); 
        adc_select_input(1);  
        adc_value_y = adc_read(); 

        uint16_t red_brightness = process_joystick_value(adc_value_x);
        uint16_t blue_brightness = process_joystick_value(adc_value_y);

        // Controle de brilho dos LEDs via PWM
        if (pwm_enabled) {
            pwm_set_gpio_level(LED_RED_PIN, red_brightness);
            pwm_set_gpio_level(LED_BLUE_PIN, blue_brightness);
        }

        // Movimentação do quadrado no display
        x_pos = (adc_value_x * 64) / 4095;
        y_pos = (adc_value_y * 128) / 4095;

        // Limita as posições para o quadrado
        if (x_pos < 4) x_pos = 4;
        if (x_pos > 52) x_pos = 52; 
        if (y_pos < 4) y_pos = 4;
        if (y_pos > 116) y_pos = 116;

        // Verifica o botão do joystick para alternar o LED verde e a borda
        bool button_pressed = !gpio_get(JOYSTICK_PB); 
        if (button_pressed && current_time - last_button_joystick > debounce_time) {
            last_button_joystick = current_time;
            toggle_led_green();
            borda_alternada = !borda_alternada; 
        }

        // Verifica o botão A para alternar o estado do PWM
        bool button_a_pressed = !gpio_get(Botao_A);
        if (button_a_pressed && current_time - last_button_a > debounce_time) {
            last_button_a = current_time;
            toggle_pwm_state();
        }

        // Limpeza do display
        ssd1306_fill(&ssd, false);

        // Desenha a borda externa
        ssd1306_rect(&ssd, 0, 0, 128, 64, cor, !cor);
        
        // Desenha a borda interna, alternando entre os estilos
        if (borda_alternada) {
            ssd1306_line(&ssd, 3, 3, 123, 3, cor); 
            ssd1306_line(&ssd, 3, 60, 123, 60, cor); 
            ssd1306_line(&ssd, 3, 3, 3, 60, cor); 
            ssd1306_line(&ssd, 123, 3, 123, 60, cor); 
        } else {
            ssd1306_line(&ssd, 3, 3, 123, 3, !cor);
            ssd1306_line(&ssd, 3, 60, 123, 60, !cor); 
            ssd1306_line(&ssd, 3, 3, 3, 60, !cor); 
            ssd1306_line(&ssd, 123, 3, 123, 60, !cor); 
        }

        // Desenha o quadrado com base no movimento do joystick
        ssd1306_rect(&ssd, x_pos, y_pos, 8, 8, cor, !cor); 
        ssd1306_send_data(&ssd);

        sleep_ms(100);
    }

    return 0;
}
