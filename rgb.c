#include "rgb.h"
#include <stdbool.h>
#include <stdlib.h>
#include "pico/stdlib.h"

#define BLUE 12    // Pino do LED azul
#define GREEN 11   // Pino do LED verde
#define RED 13 // Pino do LED vermelho

//Inicializar os pinos do led RGB
void iniciar_rgb() {
  gpio_init(RED);
  gpio_init(GREEN);
  gpio_init(BLUE);

  gpio_set_dir(RED, GPIO_OUT);
  gpio_set_dir(GREEN, GPIO_OUT);
  gpio_set_dir(BLUE, GPIO_OUT);
}

// Função para iniciar os leds RGB dos pinos 11, 12 e 13 e configurar o tempo ligado
void state(bool rr, bool gg, bool bb) {
 iniciar_rgb();
 gpio_put(RED, rr);
 gpio_put(GREEN, gg);
 gpio_put(BLUE, bb);
} 