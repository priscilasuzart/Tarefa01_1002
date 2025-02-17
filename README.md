# Tarefa01_1002
Tarefa 1 - Unidade 4 - Capítulo 8 - Conversos A/D

Este projeto utiliza um microcontrolador Raspberry Pi Pico para controlar LEDs RGB e exibir figuras em um display OLED SSD1306, com base nos movimentos de um joystick analógico.  

# Componentes necessários e suas ligações  
• LED RGB, com os pinos conectados às GPIOs (11, 12 e 13).  
• Botão do Joystick conectado à GPIO 22.  
• Joystick conectado aos GPIOs 26 e 27.  
• Botão A conectado à GPIO 5.  
• Display SSD1306 conectado via I2C (GPIO 14 e GPIO15).  

# Funcionalidades  
1. Controle de LEDs  
• Eixo X do Joystick: Controla o brilho do LED vermelho.  
• Eixo Y do Joystick: Controla o brilho do LED azul.  
• Botão do Joystick: Alterna o estado do LED verde (ligado/desligado).  
• Botão A: Alterna o estado do PWM (ativa/desativa o controle de brilho dos LEDs vermelho e azul).  
  
2. Display OLED  
• Movimento do Quadrado: O quadrado na tela se move com base nos valores dos eixos X e Y do joystick.  
• Borda Alternada: Ao pressionar o botão do joystick, o estilo da borda do display alterna entre dois estilos.

OBSERVAÇÃO: o valor central digital do joystick pode variar de placa para placa. Foi considerado uma diferença de 200 para mais ou para menos para ajustar a posição central para a placa utilizada no desenvolvimento deste projeto. Pode ser necessário um ajuste maior em outra placa.  

# Link do vídeo  
https://drive.google.com/file/d/1z-wnaLZ04bDEUOv6QHocCEj266sI36WB/view?usp=sharing  

# Desenvolvedora
Priscila Pereira Suzart de Carvalho
