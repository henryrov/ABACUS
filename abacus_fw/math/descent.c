#include <stdint.h>
#include "include.h"
#include "descent.h"
#include "../abacus.h"
#include "../sprites/sprites.h"

float evaluate_expression_from_sprites(enum sprites_e *sprites, int8_t *error_index)
{
  /* INPUT_MAX_LENGTH + 1 is the maximum number of tokens present */

  struct token_s tokens[INPUT_MAX_LENGTH + 1];

  if (tokenize(sprites, INPUT_MAX_LENGTH + 1, tokens) < 0)
    {
      *error_index = 0;
      return 0.0;
    }

  uint8_t index = 0;
  float result = parse_expression(tokens, &index, error_index);
  if (*error_index >= 0)
    {
      *error_index = token_index_to_sprite_index(tokens, *error_index);
      result = 0.0;
    }

  return result;
}
