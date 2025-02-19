#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"

#define VRX_PIN 26 // Pino X joystick
#define VRY_PIN 27 // Pino Y joystick
#define SW_PIN 22 // Pino do botão do joystick
#define LED_RED_PIN 13  // Pino do LED Vermelho (eixo X)
#define LED_BLUE_PIN 12 // Pino do LED Azul (eixo Y)
#define LED_GREEN_PIN 11  // Pino do LED Verde
#define BUTTON_A_PIN 5 // Pino do botão A
#define BUTTON_JOYSTICK_PIN 22

#define I2C_SDA 14 
#define I2C_SCL 15
#define I2C_PORT i2c1
#define endereco 0x3C

volatile bool pwm_enabled = true;

volatile uint8_t current_border = 0;

volatile uint32_t last_button_a_time = 0;
volatile uint32_t last_button_joystick_time = 0;
const uint32_t debounce_time_ms = 200;  // Tempo de debounce (200ms)

ssd1306_t ssd;

// Desenha uma borda simples no display
void draw_border_1(ssd1306_t *ssd){
    ssd1306_rect(ssd, 0, 0, 128, 64, true, false);
}

// Desenha uma borda tracejada no display
void draw_border_2(ssd1306_t *ssd) {
    for (uint8_t i = 0; i < 128; i += 4) {
        ssd1306_pixel(ssd, i, 0, true);  // Tracejado na parte superior
        ssd1306_pixel(ssd, i, 63, true); // Tracejado na parte inferior
    }
    for (uint8_t i = 0; i < 64; i += 4) {
        ssd1306_pixel(ssd, 0, i, true);  // Tracejado na parte esquerda
        ssd1306_pixel(ssd, 127, i, true); // Tracejado na parte direita
    }
}

// Atualiza o display com o valor do joystick
// desenha o quadrado na posição do joystick
void update_display(ssd1306_t *ssd, uint16_t vrx_value, uint16_t vry_value) {
    // Mapeia os valores do joystick (0-4095) para a área útil do display
    uint8_t x = (vrx_value * (64 - 8)) / 4095;
    uint8_t y = (vry_value * (128 - 8)) / 4095;

    // Limpa o display
    ssd1306_fill(ssd, false);

    
    // Desenha o quadrado
    ssd1306_rect(ssd, x, y, 8, 8, true, true);
    
    // Desenha a borda ativa
    if (current_border == 0) {
        draw_border_1(ssd);
    } else {
        draw_border_2(ssd);
    }
    // Atualiza o display
    ssd1306_send_data(ssd);
}

// Função de callback para os botões
void button_callback(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Debounce para o botão A
    if (gpio == SW_PIN && (now - last_button_a_time) > debounce_time_ms) {
        gpio_put(LED_GREEN_PIN, !gpio_get(LED_GREEN_PIN));  // Alterna o estado do LED Verde
        current_border = !current_border; // Alterna a borda do display
        last_button_a_time = now;// Atualiza o tempo do último clique
    }
    
    // Debounce para o botão do Joystick
    if (gpio == BUTTON_A_PIN && (now - last_button_joystick_time) > debounce_time_ms) {
        pwm_enabled = !pwm_enabled;  // Alterna o estado do PWM
        last_button_joystick_time = now; // Atualiza o tempo do último clique
    }
}

int main()
{

    stdio_init_all();
    adc_init(); // Inicializa o ADC
    adc_gpio_init(VRX_PIN); // Inicializa os pinos do joystick
    adc_gpio_init(VRY_PIN); // Inicializa os pinos do joystick
    gpio_init(SW_PIN); // Inicializa o pino do botão
    gpio_set_dir(SW_PIN, GPIO_IN);  // Configura o pino do botão como entrada
    gpio_pull_up(SW_PIN); // Habilita o pull-up interno

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    //Configura ssd1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Configuração para o LED Verde (botão do joystick)
    gpio_init(LED_GREEN_PIN); // Inicializa o pino do LED Verde
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT); // Configura o pino do LED Verde como saída
    gpio_put(LED_GREEN_PIN, 0); // Inicia o LED Verde desligado

    // Configuração do PWM para o LED Vermelho (eixo X)
    gpio_set_function(LED_RED_PIN, GPIO_FUNC_PWM);
    uint slice_num_red = pwm_gpio_to_slice_num(LED_RED_PIN); // Converte o pino para o slice
    uint channel_num_red = pwm_gpio_to_channel(LED_RED_PIN); // Converte o pino para o canal
    pwm_set_wrap(slice_num_red, 255);  // Define o intervalo do PWM de 0 a 255
    pwm_set_chan_level(slice_num_red, channel_num_red, 0);  // Inicia com o LED desligado
    pwm_set_enabled(slice_num_red, true); // Habilita o PWM

    // Configuração do PWM para o LED Azul (eixo Y)
    gpio_set_function(LED_BLUE_PIN, GPIO_FUNC_PWM); // Configura o pino para o PWM
    uint slice_num_blue = pwm_gpio_to_slice_num(LED_BLUE_PIN); // Converte o pino para o slice
    uint channel_num_blue = pwm_gpio_to_channel(LED_BLUE_PIN); // Converte o pino para o canal
    pwm_set_wrap(slice_num_blue, 255);  // Define o intervalo do PWM de 0 a 255
    pwm_set_chan_level(slice_num_blue, channel_num_blue, 0);  // Inicia com o LED desligado
    pwm_set_enabled(slice_num_blue, true);

    // Configuração do Botão A
    gpio_init(BUTTON_A_PIN); // Inicializa o pino do botão A
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN); // Configura o pino do botão A como entrada
    gpio_pull_up(BUTTON_A_PIN); // Habilita o pull-up interno

    // Configuração do Botão do Joystick
    gpio_init(SW_PIN); // Inicializa o pino do botão do joystick
    gpio_set_dir(SW_PIN, GPIO_IN); // Configura o pino do botão do joystick como entrada
    gpio_pull_up(SW_PIN); // Habilita o pull-up interno

    
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback); // Habilita a interrupção para o botão A
    gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback); // Habilita a interrupção para o botão do joystick
    

    while(1){
        // Pega a informação de x, y e do botão do joystick
        adc_select_input(0);
        uint16_t vrx_value = adc_read();  // Valor do eixo X
        adc_select_input(1);
        uint16_t vry_value = adc_read();  // Valor do eixo Y
        bool sw_value = gpio_get(SW_PIN) == 0;  // Valor do botão
        printf("VRX: %d, VRY: %d, SW: %d\n", vrx_value, vry_value, sw_value);

        // Mapeia o valor do eixo X para o LED Vermelho
        int16_t diff_x = (int16_t)vrx_value - 2048;  // Calcula a diferença do valor central
        uint8_t pwm_value_blue = (abs(diff_x) * 255) / 2048;  // Mapeia para 0-255
        pwm_set_chan_level(slice_num_blue, channel_num_blue, pwm_value_blue);  // Ajusta o brilho do LED Azul

        // Mapeia o valor do eixo Y para o LED Azul
        int16_t diff_y = (int16_t)vry_value - 2048;  // Calcula a diferença do valor central
        uint8_t pwm_value_red = (abs(diff_y)) * 255 / 2048;  // Mapeia para 0-255
        pwm_set_chan_level(slice_num_red, channel_num_red, pwm_value_red);  // Ajusta o brilho do LED Vermelho

        update_display(&ssd, vrx_value, vry_value); // Atualiza o display com o valor do joystick

        // Verifica se o botão foi pressionado (se o pwm está ativado)
        if(pwm_enabled){
            pwm_set_enabled(slice_num_blue, true); // Habilita o PWM do LED Azul
            pwm_set_enabled(slice_num_red, true); // Habilita o PWM do LED Vermelho
            pwm_set_chan_level(slice_num_red, channel_num_red, pwm_value_red); // Ajusta o brilho do LED Vermelho
            pwm_set_chan_level(slice_num_blue, channel_num_blue, pwm_value_blue); // Ajusta o brilho do LED Azul
        } else {
            pwm_set_enabled(slice_num_blue, false); // Desabilita o PWM do LED Azul
            pwm_set_enabled(slice_num_red, false); // Desabilita o PWM do LED Vermelho
        
        }

        sleep_ms(50);  // Aguarda 100ms antes de ler novamente
    }

    return 0;
}