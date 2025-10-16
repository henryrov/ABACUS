#ifndef ABACUS_H
#define ABACUS_H

#include <stdint.h>
#include <stdbool.h>
#include "sprites/sprites.h"

#define INPUT_MAX_LENGTH 20
#define DEFAULT_TIMEOUT 1000000
#define WAIT_WITH_TIMEOUT(condition, timeout, timeout_error) do         \
    {                                                                   \
      uint32_t i = 0;                                                   \
      while ((condition))                                               \
        {                                                               \
          i++;                                                          \
          if (i >= (timeout))                                           \
            {                                                           \
              timeout_error = true;                                     \
              break;                                                    \
            }                                                           \
        }                                                               \
                                                                        \
    } while (0)                                                         \

struct ui_state_s
{
  float result;
  enum sprites_e *sprite_string;
  int8_t error_index;
  uint8_t sprite_string_len;
  uint8_t sprite_string_i;
  int8_t left_selection_i;
  int8_t right_selection_i;
  enum sprites_e battery_level;
};

extern bool internal_error;
extern struct ui_state_s ui_state;
extern const enum sprites_e sprites_left[];
extern const uint8_t sprites_left_size;
extern const enum sprites_e sprites_right[];
extern const uint8_t sprites_right_size;

#endif
