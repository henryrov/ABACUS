#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "sprites/sprites.h"

void display_setup();
void display_move(uint8_t column, uint8_t page);
void display_draw_sprite(enum sprites_e sprite, bool invert);

#endif
