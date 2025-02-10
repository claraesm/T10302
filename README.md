Inicialização do Sistema:

O sistema inicializa o display OLED (SSD1306) e a matriz de LEDs WS2812B.
Configura os botões (A e B) para receber interrupções e realizar debounce.
Uso dos Botões:

Botão A (Pino 5): Alterna o estado do LED verde.
Botão B (Pino 6): Alterna o estado do LED azul.
Exibição no Display OLED:

Ao pressionar um botão, o display exibe mensagens indicando o LED acionado (verde ou azul) e seu novo estado (Ligado/Desligado).
Saída no Terminal (Debugging via Serial):

Mensagens no formato "Led verde ligado!" ou "Led azul desligado!" são impressas no terminal serial para monitoramento.
#   T 1 0 3 0 2  
 