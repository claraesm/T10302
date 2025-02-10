// Bibliotecas utilizadas para funcionalidades do sistema
#include "hardware/pio.h"  // Biblioteca para controlar periféricos com PIO (Programmed I/O)
#include "hardware/i2c.h" // Biblioteca para comunicação I2C
#include "pico/stdlib.h" // Biblioteca padrão para o Pico SDK
#include "ws2818b.pio.h"// Biblioteca para controlar matriz de LEDs WS2818B
#include "ssd1306.h"   // Biblioteca para controlar o display OLED SSD1306
#include <stdlib.h>   // Biblioteca padrão para funções utilitárias
#include <stdio.h>   // Biblioteca para entrada/saída padrão (printf, scanf)
#include <math.h>   // Biblioteca para operações matemáticas
#include "font.h"  // Biblioteca de fontes fornecida pelo professor Wilton
#include "rgb.h"

// Definições dos pinos para comunicação I2C
#define I2C_PORT i2c1    // Porta I2C utilizada
#define I2C_SDA 14      // Pino para SDA (Serial Data Line)
#define I2C_SCL 15     // Pino para SCL (Serial Clock Line)
#define ENDERECO 0x3C // Endereço I2C do display SSD1306

ssd1306_t ssd;  // Declara a variável ssd de forma global
// Inicialização e configurar do I2C e do display OLED SSD1306 
void display() {
    i2c_init(I2C_PORT, 400 * 1000); // Inicializa a comunicação I2C com velocidade de 400 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Configura o pino SDA para função I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura o pino SCL para função I2C
    gpio_pull_up(I2C_SDA);  // Ativa o resistor pull-up no pino SDA
    gpio_pull_up(I2C_SCL); // Ativa o resistor pull-up no pino SCL

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT); // Inicializa o display SSD1306 com dimensões e endereço I2C
    ssd1306_config(&ssd);     // Configura o display com parâmetros adicionais
    ssd1306_send_data(&ssd); // Envia dados iniciais ao display

    ssd1306_fill(&ssd, false); // Limpa o display, preenchendo com pixels apagados (falso)
    ssd1306_send_data(&ssd);  // Atualiza o display para refletir a limpeza
}

// Configurações dos botões
#define DEBOUNCE_DELAY 300 // Tempo de debounce (em milissegundos)
#define BOTAO_A 5         // Pino do Botão A
#define BOTAO_B 6        // Pino do Botão B

volatile uint32_t last_irq_time_A = 0; // Armazena o tempo da última interrupção do Botão A
volatile uint32_t last_irq_time_B = 0;// Armazena o tempo da última interrupção do Botão B

bool estado_LED_A = false;  // Estado atual do LED vermelho
bool estado_LED_B = false; // Estado atual do LED verde

void botao_callback(uint gpio, uint32_t eventos); // Declaração do callback para interrupção dos botões

// Função para inicializar os botões
void iniciar_botoes(void) {
    gpio_init(BOTAO_A); // Inicializa o pino do Botão A
    gpio_init(BOTAO_B);// Inicializa o pino do Botão B

    gpio_set_dir(BOTAO_A, GPIO_IN); // Configura o Botão A como entrada
    gpio_set_dir(BOTAO_B, GPIO_IN);// Configura o Botão B como entrada

    gpio_pull_up(BOTAO_A); // Ativa resistor pull-up no Botão A
    gpio_pull_up(BOTAO_B);// Ativa resistor pull-up no Botão B

    // Configura interrupções com callback para os botões
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, botao_callback); // Interrupção para Botão A
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, botao_callback); // Interrupção para Botão B
}

// Função para tratar debounce e alternar o estado dos LEDs
void debounce_botao(uint pino, volatile uint32_t *last_irq_time, bool *estado_LED) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time()); // Obtém o tempo atual em ms
    if (tempo_atual - *last_irq_time > DEBOUNCE_DELAY) { // Verifica se o tempo desde a última interrupção é suficiente
       
        *last_irq_time = tempo_atual;  // Atualiza o tempo da última interrupção
        *estado_LED = !(*estado_LED); // Alterna o estado do LED

        if (pino == BOTAO_A) { // Verifica se o pino é o Botão A
            bool cor = true;
            ssd1306_fill(&ssd, !cor); // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
            ssd1306_draw_string(&ssd, "Acionando o", 20, 10);  // Exibe texto no display
            ssd1306_draw_string(&ssd, "Led verde", 20, 30); // Exibe texto indicando LED vermelho
            ssd1306_draw_string(&ssd, *estado_LED ? "Ligado!" : "Desligado!", 20, 50); // Exibe estado do LED
            ssd1306_send_data(&ssd); // Atualiza o display
            printf("Led verde %s\n", *estado_LED ? "ligado!" : "desligado!"); // Log no console
            state(0,*estado_LED, 0); // Alterna o estado do LED verde

        } else if (pino == BOTAO_B) { // Verifica se o pino é o Botão B
            bool cor = true;
            ssd1306_fill(&ssd, !cor); // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
            ssd1306_draw_string(&ssd, "Acionando o", 20, 10); // Exibe texto no display
            ssd1306_draw_string(&ssd, "Led azul", 20, 30);  // Exibe texto indicando LED verde
            ssd1306_draw_string(&ssd, *estado_LED ? "Ligado!" : "Desligado!", 20, 50); // Exibe estado do LED
            ssd1306_send_data(&ssd); // Atualiza o display
            printf("Led azul %s\n", *estado_LED ? "ligado!" : "desligado!"); // Log no console
            state(0, 0,*estado_LED); // Alterna o estado do LED azul
}}}

// Callback chamado quando ocorre interrupção em algum botão
void botao_callback(uint gpio, uint32_t eventos) {
    if (gpio == BOTAO_A) { // Verifica se o botão pressionado foi o Botão A
        debounce_botao(BOTAO_A, &last_irq_time_A, &estado_LED_A);
    } else if (gpio == BOTAO_B) { // Verifica se o botão pressionado foi o Botão B
        debounce_botao(BOTAO_B, &last_irq_time_B, &estado_LED_B);
}}

//CONFIGURAÇÃO DA MATRIZ WS2812
#define PINO_MATRIZ 7 // Pino de controle da matriz de LEDs
#define NUM_LEDS 25  // Número total de LEDs na matriz
// Controlando o brilho para 30% (valor máximo é 255)
uint8_t BRILHO = 20;
// Função para converter as posições (x, y) da matriz para um índice do vetor de LEDs
int getIndex(int x, int y) {
    if (x % 2 == 0) { // Se a linha for par
        return 24 - (x * 5 + y); // Calcula o índice do LED considerando a ordem da matriz
    } else { // Se a linha for ímpar
        return 24 - (x * 5 + (4 - y)); // Calcula o índice invertendo a posição dos LEDs
}}

// Definição da estrutura de cor para cada LED
struct pixel_t {
    uint8_t R, G, B; // Componentes de cor: vermelho, verde e azul
};
typedef struct pixel_t npLED_t; // Cria um tipo npLED_t baseado em pixel_t
npLED_t leds[NUM_LEDS]; // Vetor que armazena as cores de todos os LEDs

// PIO e state machine para controle dos LEDs
PIO np_pio; // PIO utilizado para comunicação com os LEDs
uint sm;   // State machine associada ao PIO

// Função para atualizar os LEDs da matriz
void bf() {
    for (uint i = 0; i < NUM_LEDS; ++i) { // Percorre todos os LEDs
        pio_sm_put_blocking(np_pio, sm, leds[i].R); // Envia valor do componente vermelho
        pio_sm_put_blocking(np_pio, sm, leds[i].G); // Envia valor do componente verde
        pio_sm_put_blocking(np_pio, sm, leds[i].B); // Envia valor do componente azul
    }
    sleep_us(100); // Aguarda 100 microsegundos para estabilizar
}
// Função de controle inicial da matriz de LEDs
void controle(uint pino) {
    uint offset = pio_add_program(pio0, &ws2818b_program); // Carrega programa para controlar LEDs no PIO
    np_pio = pio0; // Seleciona o PIO 0
    sm = pio_claim_unused_sm(np_pio, true); // Reivindica uma state machine livre
    ws2818b_program_init(np_pio, sm, offset, pino, 800000.f); // Inicializa o controle da matriz no pino especificado

    for (uint i = 0; i < NUM_LEDS; ++i) { // Inicializa todos os LEDs com a cor preta (desligados)
        leds[i].R = leds[i].G = leds[i].B = 0;
    }
    bf(); // Atualiza o estado dos LEDs
}
// Função para configurar a cor de um LED específico
void cores(const uint indice, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[indice].R = r; // Define o componente vermelho
    leds[indice].G = g; // Define o componente verde
    leds[indice].B = b; // Define o componente azul
}
// Função para desligar todos os LEDs
void desliga() {
    for (uint i = 0; i < NUM_LEDS; ++i) { // Percorre todos os LEDs
        cores(i, 0, 0, 0); // Define a cor preta (desligado) para cada LED
    }
    bf(); // Atualiza o estado dos LEDs
}
// Função para percorrer a matriz em linha
void desenhaMatriz(int mat[5][5][3]) {
    for (int linha = 0; linha < 5; linha++) {
      for (int cols = 0; cols < 5; cols++) {
        int posicao = getIndex(linha, cols);
        cores(posicao, mat[linha][cols][0], mat[linha][cols][1], mat[linha][cols][2]);
        }}
    bf();  // Atualiza os LEDs
}

// DESENHAR OS NÚMEROS NA MATRIZ DE LEDS
void number0() {  // Função para desenhar o número 0 na matriz de LEDs
    int mat1[5][5][3] = {  // Matriz tridimensional para representar a cor de cada LED (5x5)
        // Cada elemento [linha][coluna][RGB] define a cor de um LED
        {{0, 0, 0}, {BRILHO, 0, 0}, {BRILHO, 0, 0}, {BRILHO, 0, 0}, {0, 0, 0}},   // Linha 0
        {{0, 0, 0}, {BRILHO, 0, 0}, {0, 0, 0}, {BRILHO, 0, 0},  {0, 0, 0}},      // Linha 1
        {{0, 0, 0}, {BRILHO, 0, 0}, {0, 0, 0}, {BRILHO, 0, 0},  {0, 0, 0}},     // Linha 2
        {{0, 0, 0}, {BRILHO, 0, 0}, {0, 0, 0}, {BRILHO, 0, 0},  {0, 0, 0}},    // Linha 3
        {{0, 0, 0}, {BRILHO, 0, 0}, {BRILHO, 0, 0}, {BRILHO, 0, 0}, {0, 0, 0}}// Linha 4
    };
    desenhaMatriz(mat1);
}
void number1() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, 0, 0}, {0, BRILHO, 10}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, BRILHO, 10}, {0, BRILHO, 10}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, BRILHO, 10}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, BRILHO, 10}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, BRILHO, 10}, {0, 0, 0}, {0, 0, 0}}
    };
    desenhaMatriz(mat1);
}
void number2() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, 10, BRILHO}, {0, 10, BRILHO}, {0, 10, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 10, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 10, BRILHO}, {0, 10, BRILHO}, {0, 10, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 10, BRILHO}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 10, BRILHO}, {0, 10, BRILHO}, {0, 10, BRILHO}, {0, 0, 0}}
    };
    desenhaMatriz(mat1);
}
void number3() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {100, BRILHO, 0}, {100, BRILHO, 0}, {100, BRILHO, 0}, {0, 0, 0  }},
        {{0, 0, 0}, {0, 0, 0},     {0, 0,     0}, {100, BRILHO, 0}, {0, 0,   0}},
        {{0, 0, 0}, {100, BRILHO, 0}, {100, BRILHO, 0}, {100, BRILHO, 0}, {0, 0,   0}},
        {{0, 0, 0}, {0, 0, 0},     {0, 0,     0}, {100, BRILHO, 0}, {0, 0,   0}},
        {{0, 0, 0}, {100, BRILHO, 0}, {100, BRILHO, 0}, {100, BRILHO, 0}, {0, 0,   0}}
    };
    desenhaMatriz(mat1);
}
void number4() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, 0, BRILHO}, {0, 0, 0}, {0, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, BRILHO}, {0, 0, 0}, {0, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, BRILHO}, {0, 0, BRILHO}, {0, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, BRILHO}, {0, 0, 0}}
    };
    desenhaMatriz(mat1);
}
void number5() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, BRILHO, 0}, {0, BRILHO, 0}, {0, BRILHO, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, BRILHO, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, BRILHO, 0}, {0, BRILHO, 0}, {0, BRILHO, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, BRILHO, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, BRILHO, 0}, {0, BRILHO, 0}, {0, BRILHO, 0}, {0, 0, 0}}
    };
    desenhaMatriz(mat1);
}
void number6() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, 0, BRILHO}, {0, 0, BRILHO}, {0, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, BRILHO}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, BRILHO}, {0, 0, BRILHO}, {0, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, BRILHO}, {0, 0, 0}, {0, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, BRILHO}, {0, 0, BRILHO}, {0, 0, BRILHO}, {0, 0, 0}}
    };
    desenhaMatriz(mat1);
}
void number7() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {0, BRILHO, BRILHO}, {0, BRILHO, BRILHO}, {0, BRILHO, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, BRILHO, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, BRILHO, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, BRILHO, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, BRILHO, BRILHO}, {0, 0, 0}}
    };
    desenhaMatriz(mat1);
}
void number8() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {BRILHO, 0, BRILHO}, {BRILHO, 0, BRILHO}, {BRILHO, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {BRILHO, 0, BRILHO}, {0, 0, 0}, {BRILHO, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {BRILHO, 0, BRILHO}, {BRILHO, 0, BRILHO}, {BRILHO, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {BRILHO, 0, BRILHO}, {0, 0, 0}, {BRILHO, 0, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {BRILHO, 0, BRILHO}, {BRILHO, 0, BRILHO}, {BRILHO, 0, BRILHO}, {0, 0, 0}}
    };
    desenhaMatriz(mat1);
}
void number9() {
    int mat1[5][5][3] = {
        {{0, 0, 0}, {BRILHO, BRILHO, BRILHO}, {BRILHO, BRILHO, BRILHO}, {BRILHO, BRILHO, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {BRILHO, BRILHO, BRILHO}, {0, 0, 0}, {BRILHO, BRILHO, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {BRILHO, BRILHO, BRILHO}, {BRILHO, BRILHO, BRILHO}, {BRILHO, BRILHO, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {BRILHO, BRILHO, BRILHO}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {BRILHO, BRILHO, BRILHO}, {0, 0, 0}}
    };
    desenhaMatriz(mat1);
}

char atualizar = 0; // Estado atual do sistema
void comando(char atualizar) {
    switch (atualizar) {
        case '0': number0(); break;
        case '1': number1(); break;
        case '2': number2(); break;
        case '3': number3(); break;
        case '4': number4(); break;
        case '5': number5(); break;
        case '6': number6(); break;
        case '7': number7(); break;
        case '8': number8(); break;
        case '9': number9(); break;
        
        default:  desliga();  state(0,0,0); break;
}}

int main() {
    controle(PINO_MATRIZ);// Configura controle na matriz
    stdio_init_all();    // Inicializa entrada/saída padrão
    iniciar_botoes();   // Inicialização dos botões
    iniciar_rgb();     // LEDs RGB 
    display();        // Inicializa o display
    bool cor = true; // Declarado corretamente antes de seu uso

      // Exibição inicial no display OLED
      ssd1306_fill(&ssd, !cor);                             // Limpa o display preenchendo com a cor oposta ao valor atual de "cor"
      ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);        // Desenha um retângulo com bordas dentro das coordenadas especificadas
      ssd1306_draw_string(&ssd, "Iniciando", 27, 10);       // Escreve "Utilize" na posição (42, 10) do display
      ssd1306_draw_string(&ssd, "Use os botoes", 17, 30);// Escreve "os botoes" na posição (17, 30) do display
      ssd1306_draw_string(&ssd, "ou o teclado", 20, 50);// Escreve "ou o teclado" na posição (20, 50) do display
      ssd1306_send_data(&ssd);                         // Envia os dados para atualizar o display

// Loop infinito para manter a aplicação em execução
while (true) {
    cor = !cor;  // Alterna a cor (inverte o valor de "cor")
    ssd1306_send_data(&ssd);  // Atualiza o display
    sleep_ms(500);  // Aguarda 500 ms

    printf("Digite um caractere: ");                         // Solicita a entrada de um caractere ao usuário
    fflush(stdout);                                         // Certifica-se de que o buffer de saída seja limpo antes de aguardar a entrada
    if (scanf(" %c", &atualizar) == 1) {                   // Lê um caractere do usuário e verifica se a leitura foi bem-sucedida
        comando(atualizar);                               // Executa um comando com base no caractere digitado
        printf("%c\n", atualizar);                       // Exibe o caractere digitado
        ssd1306_fill(&ssd, !cor);                       // Limpa o display com a cor oposta
        ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);  // Desenha um retângulo com bordas no display
        ssd1306_draw_string(&ssd, &atualizar, 63, 29);// Mostra o caractere digitado na posição (63, 29) do display
        ssd1306_send_data(&ssd);                     // Atualiza o display com as novas informações
    }}
    return 0;  // Retorna 0 ao sistema operacional (nunca será alcançado devido ao loop infinito)
}