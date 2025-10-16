#include <math.h>
#include <stdbool.h>
#include "abacus.h"
#include "interface.h"
#include "display.h"
#include "sprites/sprites.h"

#define INPUT_MAX_DISPLAY_WIDTH 10

void scale_result(float in,
                  enum sprites_e *result_sprites,
                  enum sprites_e *unit_prefix,
                  enum sprites_e *sign_sprite)
{
  if (in == 0.0)
    {
      result_sprites[0] = SPRITE_0;
      result_sprites[1] = SPRITE_DOT;
      result_sprites[2] = SPRITE_0;
      result_sprites[3] = SPRITE_0;
      result_sprites[4] = SPRITE_0;
      *unit_prefix = SPRITE_BLANK;
      *sign_sprite = SPRITE_BLANK;
      return;
    }
  else
    {
      if (in < 0)
        {
          *sign_sprite = SPRITE_MINUS;
          in = -1.0 * in;
        }
      else
        {
          *sign_sprite = SPRITE_BLANK;
        }

      int8_t pow_10 = 0;
      uint8_t digits_before_decimal;
      int8_t prefix_pow;

      if (in >= 1)
        {
          while ((int)(in / pow(10.0, pow_10)))
            {
              pow_10++;
            }

          pow_10--;
        }
      else
        {
          while ((int)(in / pow(10.0, pow_10)) == 0)
            {
              pow_10--;
            }
        }

      float factor_to_integer = powf(10.0, -1 * pow_10) * 1000;
      if ((uint16_t)(round(in * factor_to_integer)) == 10000)
        {
          /* In some cases, rounding may increase pow_10 (e.g. rounding up 0.99999) */

          pow_10++;
        }

      float rounded_in = round(in * factor_to_integer) / factor_to_integer;

      if (rounded_in >= 1)
        {
          digits_before_decimal = (pow_10 % 3) + 1;
          prefix_pow = (pow_10 / 3) * 3;
        }
      else
        {
          switch (pow_10 % 3)
            {
            case 0:
              digits_before_decimal = 1;
              break;

            case -1:
              digits_before_decimal = 3;
              break;

            case -2:
              digits_before_decimal = 2;
              break;
            }

          prefix_pow = pow_10;
          while ((prefix_pow % 3) != 0)
            {
              prefix_pow--;
            }
        }

      if (prefix_pow > 9)
        {
          /* For now refuse to return anything this big */

          result_sprites[0] = SPRITE_UC_B;
          result_sprites[1] = SPRITE_UC_I;
          result_sprites[2] = SPRITE_UC_G;
          result_sprites[3] = SPRITE_EXCLAMATION;
          result_sprites[4] = SPRITE_EXCLAMATION;
          *unit_prefix = SPRITE_BLANK;
          return;
        }
      else if (prefix_pow < -12)
        {
          /* Also refuse to return anything tiny */

          result_sprites[0] = SPRITE_UC_T;
          result_sprites[1] = SPRITE_UC_I;
          result_sprites[2] = SPRITE_UC_N;
          result_sprites[3] = SPRITE_UC_Y;
          result_sprites[4] = SPRITE_EXCLAMATION;
          *unit_prefix = SPRITE_BLANK;
          return;
        }

      result_sprites[digits_before_decimal] = SPRITE_DOT;

      for (int i = 0; i < 4; i++)
        {
          enum sprites_e digit = (enum sprites_e)
            ((int)(rounded_in / pow(10.0, pow_10 - i)) % 10);

          if (i < digits_before_decimal)
            {
              result_sprites[i] = digit;
            }
          else
            {
              result_sprites[i + 1] = digit;
            }
        }

      switch (prefix_pow)
        {
        case -12:
          *unit_prefix = SPRITE_LC_P;
          break;

        case -9:
          *unit_prefix = SPRITE_LC_N;
          break;

        case -6:
          *unit_prefix = SPRITE_LC_MU;
          break;

        case -3:
          *unit_prefix = SPRITE_LC_M;
          break;

        case 3:
          *unit_prefix = SPRITE_LC_K;
          break;

        case 6:
          *unit_prefix = SPRITE_UC_M;
          break;

        case 9:
          *unit_prefix = SPRITE_UC_G;
          break;

        default:
          *unit_prefix = SPRITE_BLANK;
          break;
        }
    }
}

void interface_draw_result()
{
  enum sprites_e result_sprites[5];
  enum sprites_e unit_prefix;
  enum sprites_e sign_sprite;

  scale_result(ui_state.result, result_sprites, &unit_prefix, &sign_sprite);

  display_move(24, 5);
  display_draw_sprite(sign_sprite, false);
  for (uint8_t i = 0; i < 5; i++)
    {
      display_draw_sprite(result_sprites[i], false);
    }

  display_move(80, 5);
  display_draw_sprite(unit_prefix, false);
}

void interface_draw_input()
{
  uint8_t display_column = 24;
  uint32_t start_index = 0;
  if (ui_state.sprite_string_i > INPUT_MAX_DISPLAY_WIDTH - 1)
    {
      start_index = ui_state.sprite_string_i - INPUT_MAX_DISPLAY_WIDTH + 1;
    }

  display_move(24, 4);
  for (uint8_t i = 0; i < INPUT_MAX_DISPLAY_WIDTH; i++)
    {
      display_draw_sprite(SPRITE_BLANK, false);
    }

  display_move(24, 3);
  for (uint8_t i = start_index;
       i < (start_index + INPUT_MAX_DISPLAY_WIDTH);
       i++)
    {
      bool invert = (i == ui_state.sprite_string_i);
      display_draw_sprite(ui_state.sprite_string[i], invert);
      if (i == ui_state.error_index)
        {
          display_move(display_column, 4);
          display_draw_sprite(SPRITE_ARROW_UP, false);
          display_move(display_column + 8, 3);
        }

      display_column += 8;
    }
}

void interface_draw_selection(enum selection_side_e side)
{
  const enum sprites_e *selection;
  uint8_t selection_size;
  uint8_t start_column;
  uint8_t index;
  switch (side)
    {
    case SIDE_LEFT:
      selection = sprites_left;
      selection_size = sprites_left_size;
      start_column = 0;
      index = ui_state.left_selection_i;
      break;

    case SIDE_RIGHT:
      selection = sprites_right;
      selection_size = sprites_right_size;
      start_column = 120;
      index = ui_state.right_selection_i;
      break;
    }

  display_move(start_column, 4);
  display_draw_sprite(selection[index], true);
  if (index > 0)
    {
      display_move(start_column, 3);
      display_draw_sprite(selection[index - 1], false);
    }
  else
    {
      display_move(start_column, 3);
      display_draw_sprite(SPRITE_BLANK, false);
    }

  if (index < (selection_size - 1))
    {
      display_move(start_column, 5);
      display_draw_sprite(selection[index + 1], false);
    }
  else
    {
      display_move(start_column, 5);
      display_draw_sprite(SPRITE_BLANK, false);
    }
}

void interface_draw_battery()
{
  display_move(120, 0);
  display_draw_sprite(ui_state.battery_level, false);
}

void interface_draw_error()
{
  if (internal_error)
    {
      display_move(120, 7);
      display_draw_sprite(SPRITE_UC_E, false);
    }
  else
    {
      display_move(120, 7);
      display_draw_sprite(SPRITE_BLANK, false);
    }
}
