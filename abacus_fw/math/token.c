#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "include.h"
#include "../abacus.h"
#include "../sprites/sprites.h"

float sprites_to_f(enum sprites_e *sprites)
{
  float result = 0.0;
  bool decimal_present = false;

  uint8_t integers_len = 0;
  while ((sprites[integers_len] != SPRITE_BLANK)
         && (sprites[integers_len] != SPRITE_DOT))
    {
      integers_len++;
    }

  if (sprites[integers_len] == SPRITE_DOT)
    {
      decimal_present = true;
    }

  /* Find the integer part */

  for (uint8_t i = 0; i < integers_len; i++)
    {
      /* Check for invalid sprites */

      if (sprites[i] > SPRITE_9)
        {
          internal_error = true;
          return 0.0;
        }

      result += (float)(sprites[i] - SPRITE_0) * powf(10.0, integers_len - 1 - i);
    }

  if (decimal_present)
    {
      /* Find the decimal part */

      for (uint8_t i = integers_len + 1; sprites[i] != SPRITE_BLANK; i++)
        {
          /* Check for invalid sprites */

          if (sprites[i] > SPRITE_9)
            {
              internal_error = true;
              return 0.0;
            }

          result += (float)(sprites[i] - SPRITE_0) * powf(10.0, integers_len - i);
        }
    }

  return result;
}

int8_t tokenize(enum sprites_e *input_sprites, uint8_t sprites_len, struct token_s *token_list)
{
  uint8_t token_index = 0;
  struct token_s *current_token = &token_list[token_index];

  enum sprites_e working_sprites[sprites_len];
  uint8_t working_index = 0;

  for (uint8_t i = 0; i < sprites_len; i++)
    {
      char current_sprite = input_sprites[i];

      if (((current_sprite >= SPRITE_0) && (current_sprite <= SPRITE_9))
          || (current_sprite == SPRITE_DOT))
        {
          /* Sprite is part of a number */

          working_sprites[working_index] = current_sprite;
          working_index++;
        }
      else if ((current_sprite == SPRITE_UC_A) || (current_sprite == SPRITE_LC_E)
               || (current_sprite == SPRITE_LC_PI))
        {
          /* Sprite is a symbol representing a number */

          if (working_index != 0)
            {
              /* Terminate previously entered number */

              working_sprites[working_index] = SPRITE_BLANK;
              current_token->type = NUMBER;
              current_token->value = sprites_to_f(working_sprites);
              current_token->length = working_index;
              current_token->symbol = SPRITE_BLANK;

              working_index = 0;

              token_index++;
              current_token = &token_list[token_index];
            }

          current_token->type = NUMBER;
          float value;
          switch (current_sprite)
            {
            case SPRITE_UC_A:
              value = ui_state.result;
              break;

            case SPRITE_LC_E:
              value = M_E;
              break;

            case SPRITE_LC_PI:
              value = M_PI;
              break;

            default:
              value = 0;
              break;
            }

          current_token->value = value;
          current_token->length = 1;
          current_token->symbol = SPRITE_BLANK;

          token_index++;
          current_token = &token_list[token_index];
        }
      else if (current_sprite == SPRITE_PLUS || current_sprite == SPRITE_MINUS
               || current_sprite == SPRITE_TIMES || current_sprite == SPRITE_DIVIDE
               || current_sprite == SPRITE_POWER
               || current_sprite == SPRITE_LEFT_PARENTHESIS
               || current_sprite == SPRITE_RIGHT_PARENTHESIS)
        {
          if (working_index != 0)
            {
              /* Terminate previously entered number */

              working_sprites[working_index] = SPRITE_BLANK;
              current_token->type = NUMBER;
              current_token->value = sprites_to_f(working_sprites);
              current_token->length = working_index;
              current_token->symbol = SPRITE_BLANK;

              working_index = 0;

              token_index++;
              current_token = &token_list[token_index];
            }

          current_token->type = SYMBOL;
          current_token->symbol = current_sprite;
          current_token->value = 0.0;
          current_token->length = 1;

          token_index++;
          current_token = &token_list[token_index];
        }
      else
        {
          /* Unrecognized sprite */

          continue;
        }
    }

  if (working_index != 0)
    {
      if (working_index >= sprites_len)
        {
          working_sprites[sprites_len - 1] = SPRITE_BLANK;
        }
      else
        {
          working_sprites[working_index] = SPRITE_BLANK;
        }

      current_token->type = NUMBER;
      current_token->value = sprites_to_f(working_sprites);
      current_token->length = working_index;
      current_token->symbol = SPRITE_BLANK;

      token_index++;
      current_token = &token_list[token_index];
    }

  current_token->type = END;
  current_token->symbol = SPRITE_BLANK;
  current_token->value = 0.0;
  current_token->length = 1;

  return 0;
}

int8_t token_index_to_sprite_index(struct token_s *tokens, int8_t token_index)
{
  if (token_index < 0)
    {
      return -1;
    }
  else
    {
      int8_t sprite_index = 0;
      for (int i = 0; i < token_index; i++)
        {
          sprite_index += tokens[i].length;
        }

      return sprite_index;
    }
}
