# Joystick com Raspberry Pi Pico e Display OLED SSD1306

Este projeto utiliza um Raspberry Pi Pico para controlar um joystick analógico, com saída para um display OLED SSD1306 via I2C. Além disso, LEDs indicam a posição do joystick por meio de PWM.

## Funcionalidades

- Leitura do Joystick: Captura os valores dos eixos X e Y usando o ADC do Raspberry Pi Pico.

- Exibição no Display OLED: O display SSD1306 exibe um quadrado móvel, representando a posição do joystick.

- Alternância de Bordas no Display: Pressionar o botão do joystick altera entre dois estilos de borda.

- Controle de LEDs RGB via PWM:

  - O LED vermelho responde ao eixo Y.

  - O LED azul responde ao eixo X.

- Interrupções para Botões:

   - O botão do joystick alterna a borda do display e aciona um LED verde.

   - Um botão adicional ativa/desativa os LEDs RGB.

## Dependências

- Pico SDK

- Biblioteca SSD1306 (Inclusa no diretório lib/)

## Demonstração

https://youtu.be/CQ6fZgJdxGc
