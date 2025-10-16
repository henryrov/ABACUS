#include <stdint.h>
#include "battery.h"
#include "sprites/sprites.h"

#define ADC_MAX_VAL 4095
#define BATTERY_MIN_VOLTAGE 3.3
#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_THRESHOLD(v) (ADC_MAX_VAL * 0.5 / 3.3                   \
                              * (BATTERY_MIN_VOLTAGE +                  \
                                 (v) * (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)))

enum sprites_e get_battery_level(uint16_t adc_battery)
{
  enum sprites_e battery_level;
  
  if (adc_battery > BATTERY_THRESHOLD(1.0))
    {
      battery_level = SPRITE_BATTERY_100;
    }
  else if (adc_battery > BATTERY_THRESHOLD(0.80))
    {
      battery_level = SPRITE_BATTERY_80;
    }
  else if (adc_battery > BATTERY_THRESHOLD(0.60))
    {
      battery_level = SPRITE_BATTERY_60;
    }
  else if (adc_battery > BATTERY_THRESHOLD(0.40))
    {
      battery_level = SPRITE_BATTERY_40;
    }
  else if (adc_battery > BATTERY_THRESHOLD(0.20))
    {
      battery_level = SPRITE_BATTERY_20;
    }
  else
    {
      battery_level = SPRITE_BATTERY_0;
    }

  return battery_level;
}
