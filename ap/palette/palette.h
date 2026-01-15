
#include "stdint.h"

#define PALETTE_SIZE    256

typedef uint16_t PALETTE[PALETTE_SIZE];

void palette_generate(PALETTE * palette);

int palette_get_distance_RGB565(uint16_t color1, uint16_t color2);

uint8_t palette_get_color_idx(uint16_t color, PALETTE * palette);

