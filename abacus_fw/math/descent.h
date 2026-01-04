#ifndef DESCENT_H
#define DESCENT_H

#include <stdint.h>
#include <math.h>
#include "include.h"
#include "../sprites/sprites.h"

/******************** Library public funtions ************************/

/*********************************************************************
 * Name: evaluate_expression_from_string
 *
 * Description: Takes in a string describing an algebraic expression
 *              and attempts to evaluate it. If an error occurs,
 *              its (token) location is placed in error_index.
 *              The value at error_index should be negative before
 *              this function is called.
 *
 * Returns: The result of the expression if successful, 0 otherwise.
 *********************************************************************/

float evaluate_expression_from_sprites(enum sprites_e *sprites, int8_t *error_index);

#endif
