#ifndef INCLUDE_H
#define INCLUDE_H

#include <stdint.h>
#include "../sprites/sprites.h"

/* Types */

enum token_type_e
{
  NUMBER,
  SYMBOL,
  END
};

struct token_s
{
  enum token_type_e type;
  float value;
  uint8_t length;
  enum sprites_e symbol;
};

/* Functions */

int8_t tokenize(enum sprites_e *sprites, uint8_t string_len, struct token_s *token_list);
int8_t token_index_to_sprite_index(struct token_s *tokens, int8_t token_index);
float parse_expression(struct token_s *tokens, uint8_t *index, int8_t *error_index);

#endif
