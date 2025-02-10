#ifndef RGB_H
#define RGB_H

#include <stdbool.h>
#include "pico/stdlib.h"  // Biblioteca necessária para GPIO no Raspberry Pi Pico

// Definições dos pinos do LED RGB
#define BLUE 12
#define GREEN 11
#define RED 13

// Protótipos das funções
void iniciar_rgb(void);
void state(bool rr, bool gg, bool bb);

#endif // RGB_H
