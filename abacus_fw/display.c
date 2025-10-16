#include <stdbool.h>
#include "abacus.h"
#include "display.h"
#include "stm32c0.h"
#include "sprites/sprites.h"

#define DISPLAY_ON 0xAF
#define DISPLAY_SET_COLUMN_LN 0x00
#define DISPLAY_SET_COLUMN_UN 0x10
#define DISPLAY_SET_PAGE 0xB0

enum transfer_type_e
{
  TRANSFER_COMMAND,
  TRANSFER_DATA,
  TRANSFER_RAW
};

void i2c_write(enum transfer_type_e type,
               const uint8_t *bytes,
               uint8_t n_bytes,
               bool invert)
{
  bool timeout_error = false;

  WAIT_WITH_TIMEOUT(I2C1_ISR & (1 << 15), DEFAULT_TIMEOUT, timeout_error);
  if (timeout_error)
    {
      internal_error = true;
      return;
    }

  I2C1_CR2 &= ~(0xff << 16);
  if (type == TRANSFER_RAW)
    {
      I2C1_CR2 |= n_bytes << 16;
    }
  else
    {
      I2C1_CR2 |= (n_bytes + 1) << 16;
    }

  I2C1_CR2 |= 1 << 13;

  WAIT_WITH_TIMEOUT(!(I2C1_ISR & (1 << 1)), DEFAULT_TIMEOUT, timeout_error);
  if (timeout_error)
    {
      internal_error = true;
      return;
    }

  if (type == TRANSFER_COMMAND)
    {
      I2C1_TXDR = 0x00;
    }
  else if (type == TRANSFER_DATA)
    {
      I2C1_TXDR = 0x40;
    }

  for (uint8_t i = 0; i < n_bytes; i++)
    {
      WAIT_WITH_TIMEOUT(!(I2C1_ISR & (1 << 1)), DEFAULT_TIMEOUT, timeout_error);
      if (timeout_error)
        {
          internal_error = true;
          return;
        }

      if (invert)
        {
          I2C1_TXDR = ~bytes[i];
        }
      else
        {
          I2C1_TXDR = bytes[i];
        }
    }
}

void display_setup()
{
  uint8_t bytes[3];
  bytes[0] = 0x8D;
  bytes[1] = 0x14;
  bytes[2] = DISPLAY_ON;
  i2c_write(TRANSFER_COMMAND, bytes, 3, false);

  /* Clear display RAM */

  for (uint8_t page = 0; page < 8; page++)
    {
      display_move(0, page);
      for (uint8_t column = 0; column < 128; column = column + 8)
        {
          display_draw_sprite(SPRITE_BLANK, false);
        }
    }

  display_move(0, 0);
}

void display_move(uint8_t column, uint8_t page)
{
  uint8_t bytes[1];
  bytes[0] = DISPLAY_SET_COLUMN_LN + (column & 0x0F);
  i2c_write(TRANSFER_COMMAND, bytes, 1, false);
  bytes[0] = DISPLAY_SET_COLUMN_UN + ((column >> 4) & 0x0F);
  i2c_write(TRANSFER_COMMAND, bytes, 1, false);
  bytes[0] = DISPLAY_SET_PAGE + page;
  i2c_write(TRANSFER_COMMAND, bytes, 1, false);
}

void display_draw_sprite(enum sprites_e sprite, bool invert)
{
  i2c_write(TRANSFER_DATA, sprites[(uint8_t)sprite], 8, invert);
}
