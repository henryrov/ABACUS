#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>
#include "sprites/sprites.h"

enum sprites_e get_battery_level(uint16_t adc_battery);

#endif
