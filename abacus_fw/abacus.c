#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "abacus.h"
#include "stm32c0.h"
#include "interface.h"
#include "display.h"
#include "battery.h"
#include "sprites/sprites.h"
#include "math/descent.h"

#define enable_interrupts() asm(" cpsie i ")
#define enter_sleep() asm(" wfi ")

#define DISPLAY_ADDR 0x3C
#define ADC_MAX_VAL 4095
#define ADC_BUTTON_THRESHOLD 250
#define ADC_SLIDER_MIN ((ADC_MAX_VAL) * 1000/12000)
#define ADC_SLIDER_MAX ((ADC_MAX_VAL) * 11000/12000)

void default_isr();
void tim14_isr();

enum sprites_e input_string[INPUT_MAX_LENGTH + 1];
bool internal_error = false;
const enum sprites_e sprites_left[] =
{
  SPRITE_0,
  SPRITE_1,
  SPRITE_2,
  SPRITE_3,
  SPRITE_4,
  SPRITE_5,
  SPRITE_6,
  SPRITE_7,
  SPRITE_8,
  SPRITE_9,
  SPRITE_DOT,
  SPRITE_UC_A,
  SPRITE_LC_E,
  SPRITE_LC_PI
};

const uint8_t sprites_left_size = sizeof(sprites_left) / sizeof(enum sprites_e);

const enum sprites_e sprites_right[] =
{
  SPRITE_UC_C,
  SPRITE_UC_D,
  SPRITE_ARROW_LEFT,
  SPRITE_ARROW_RIGHT,
  SPRITE_LEFT_PARENTHESIS,
  SPRITE_RIGHT_PARENTHESIS,
  SPRITE_PLUS,
  SPRITE_MINUS,
  SPRITE_TIMES,
  SPRITE_DIVIDE,
  SPRITE_POWER,
  SPRITE_EQUALS
};

const uint8_t sprites_right_size = sizeof(sprites_right) / sizeof(enum sprites_e);

bool left_button_ready = true;
bool right_button_ready = true;
uint8_t prev_left_selection_i = 0;
uint8_t prev_right_selection_i = 0;
enum sprites_e prev_battery_level = SPRITE_BATTERY_0;
struct ui_state_s ui_state =
{
  .result = 0.0,
  .error_index = -1,
  .sprite_string = input_string,
  .sprite_string_len = 0,
  .sprite_string_i = 0,
  .left_selection_i = 0,
  .right_selection_i = 0,
  .battery_level = SPRITE_BATTERY_0
};


const void *vectors[] __attribute__((section(".init"))) =
{
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  default_isr,
  tim14_isr,
};

void sprite_string_try_insert(enum sprites_e sprite)
{
  if (ui_state.sprite_string_i < INPUT_MAX_LENGTH)
    {
      ui_state.sprite_string[ui_state.sprite_string_i] = sprite;

      if (ui_state.sprite_string_i == ui_state.sprite_string_len)
        {
          /* Inserting at the end of the string */

          ui_state.sprite_string_len++;
        }

      ui_state.sprite_string_i++;
    }
}

void sprite_string_delete()
{
  if ((ui_state.sprite_string_i == ui_state.sprite_string_len)
      && (ui_state.sprite_string_i > 0))
    {
      /* Deleting rightmost sprite */

      ui_state.sprite_string_i--;
      ui_state.sprite_string[ui_state.sprite_string_i] = SPRITE_BLANK;
    }
  else
    {
      /* Move everything that comes after back one index */

      for (uint8_t i = ui_state.sprite_string_i;
           ui_state.sprite_string[i] != SPRITE_BLANK;
           i++)
        {
          ui_state.sprite_string[i] = ui_state.sprite_string[i + 1];
        }
    }

  ui_state.sprite_string_len--;
}

void default_isr()
{
  /* This should never run */

  while (1);
}

void tim14_isr()
{
  bool timeout_error = false;
  internal_error = false;

  /* Read ADC to determine selection or button presses */

  WAIT_WITH_TIMEOUT(!(ADC_ISR & 1), DEFAULT_TIMEOUT, timeout_error);
  if (timeout_error)
    {
      internal_error = true;
      TIM14_SR &= ~(1);
      return;
    }

  ADC_CR |= 1 << 2;
  WAIT_WITH_TIMEOUT(!(ADC_ISR & (1 << 2)), DEFAULT_TIMEOUT, timeout_error);
  if (timeout_error)
    {
      internal_error = true;
      TIM14_SR &= ~(1);
      return;
    }

  uint16_t adc_right = ADC_DR;
  ADC_CR |= 1 << 2;
  WAIT_WITH_TIMEOUT(!(ADC_ISR & (1 << 2)), DEFAULT_TIMEOUT, timeout_error);
  if (timeout_error)
    {
      internal_error = true;
      TIM14_SR &= ~(1);
      return;
    }

  uint16_t adc_left = ADC_DR;
  ADC_CR |= 1 << 2;
  WAIT_WITH_TIMEOUT(!(ADC_ISR & (1 << 2)), DEFAULT_TIMEOUT, timeout_error);
  if (timeout_error)
    {
      internal_error = true;
      TIM14_SR &= ~(1);
      return;
    }

  uint16_t adc_bat = ADC_DR;
  enum sprites_e new_battery_level = get_battery_level(adc_bat);
  if (new_battery_level != prev_battery_level)
    {
      ui_state.battery_level = new_battery_level;
      interface_draw_battery();
      prev_battery_level = new_battery_level;
    }

  bool input_changed = false;

  if ((adc_right <= ADC_BUTTON_THRESHOLD) && right_button_ready)
    {
      /* Right button pressed */

      switch (sprites_right[ui_state.right_selection_i])
        {
        case SPRITE_EQUALS:
          int8_t error = -1;
          float result = evaluate_expression_from_sprites(ui_state.sprite_string,
                                                          &error);
          ui_state.error_index = error;
          input_changed = true;
          if (error < 0)
            {
              ui_state.result = result;
              interface_draw_result();
            }
          break;

        case SPRITE_UC_C:
          /* Clear input string */

          ui_state.sprite_string_i = 0;
          ui_state.sprite_string_len = 0;
          input_changed = true;
          for (int i = 0; i <= INPUT_MAX_LENGTH; i++)
            {
              input_string[i] = SPRITE_BLANK;
            }
          break;

        case SPRITE_UC_D:
          sprite_string_delete();
          input_changed = true;
          break;

        case SPRITE_ARROW_LEFT:
          if (ui_state.sprite_string_i > 0)
            {
              ui_state.sprite_string_i--;
              input_changed = true;
            }
          break;

        case SPRITE_ARROW_RIGHT:
          if (ui_state.sprite_string_i < ui_state.sprite_string_len)
            {
              ui_state.sprite_string_i++;
              input_changed = true;
            }
          break;

        default:
          sprite_string_try_insert(sprites_right[ui_state.right_selection_i]);
          input_changed = true;
          break;
        }

      right_button_ready = false;
    }
  else if (adc_right > ADC_BUTTON_THRESHOLD)
    {
      uint16_t selection_separation = (ADC_SLIDER_MAX - ADC_SLIDER_MIN)
        / sprites_right_size;
      ui_state.right_selection_i = sprites_right_size - 1
        - (uint8_t)((adc_right - ADC_SLIDER_MIN) / selection_separation);
      if (ui_state.right_selection_i >= sprites_right_size)
        {
          ui_state.right_selection_i = sprites_right_size - 1;
        }
      else if (ui_state.right_selection_i < 0)
        {
          ui_state.right_selection_i = 0;
        }

      right_button_ready = true;
    }

  if ((adc_left <= ADC_BUTTON_THRESHOLD) && left_button_ready)
    {
      /* Left button pressed */

      sprite_string_try_insert(sprites_left[ui_state.left_selection_i]);
      input_changed = true;

      left_button_ready = false;
    }
  else if (adc_left > ADC_BUTTON_THRESHOLD)
    {
      uint16_t selection_separation = (ADC_SLIDER_MAX - ADC_SLIDER_MIN)
        / sprites_left_size;
      ui_state.left_selection_i = sprites_left_size - 1
        - (uint8_t)((adc_left - ADC_SLIDER_MIN) / selection_separation);
      if (ui_state.left_selection_i >= sprites_left_size)
        {
          ui_state.left_selection_i = sprites_left_size - 1;
        }
      else if (ui_state.left_selection_i < 0)
        {
          ui_state.left_selection_i = 0;
        }

      left_button_ready = true;
    }

  if (input_changed)
    {
      interface_draw_input();
    }

  if (ui_state.right_selection_i != prev_right_selection_i)
    {
      interface_draw_selection(SIDE_RIGHT);
      prev_right_selection_i = ui_state.right_selection_i;
    }

  if (ui_state.left_selection_i != prev_left_selection_i)
    {
      interface_draw_selection(SIDE_LEFT);
      prev_left_selection_i = ui_state.left_selection_i;
    }

  interface_draw_error();

  TIM14_SR &= ~(1);
}

void init()
{
  /* Peripheral clock setup */

  RCC_IOPENR = 0b11;
  RCC_APBENR1 = 1 << 21;
  RCC_APBENR2 = (1 << 15) | (1 << 20) | 1;

  /* Delay to make sure clocks are active and the display is powered,
   * and to allow debugging.
   */

  for (int i = 0; i < 2000000; i++);

  /* Remap PA12 to PA10 */

  SYSCFG_CFGR3 |= 1 << 8;
  SYSCFG_CFGR1 |= 1 << 4;

  /* ADC setup - calibration then initialization */

  ADC_CR |= 1 << 28;
  for (int i = 0; i < 100000; i++);
  ADC_CR |= 1 << 31;
  while (ADC_CR & (1 << 31));
  uint8_t calibration_factor = (ADC_DR & (0x7f)) + 1;

  ADC_CHSELR = (1 << 14) | (1 << 13) | (1 << 11);
  ADC_CFGR1 = 1 << 16;
  ADC_CR |= 1;
  while (!(ADC_ISR & 1));
  ADC_CALFACT = calibration_factor;
  ADC_SMPR |= 0b111;

  /* GPIO pin setup */

  GPIOX_MODER('A') = 0xFFEFFFFF;
  GPIOX_AFRH('A') = 0x00000600;
  GPIOX_MODER('B') = 0xFFFFBFFF;
  GPIOX_AFRL('B') = 0xE0000000;
  GPIOX_OTYPER('A') |= 1 << 10;
  GPIOX_OTYPER('B') |= 1 << 7;
  GPIOX_PUPDR('A') &= ~(0b11 << 26) & ~(0b11 << 28);
  GPIOX_PUPDR('A') |= 0b01 << 20;
  GPIOX_PUPDR('B') |= 0b01 << 14;

  /* I2C setup */

  I2C1_TIMINGR = (10) | (10 << 8) | (1 << 16) | (3 << 20);
  I2C1_CR1 |= 1;
  while (!(I2C1_CR1 & 1));
  I2C1_CR2 = (DISPLAY_ADDR << 1) | (1 << 25);

  display_setup();

  /* Timer setup */

  TIM14_PSC = 11999;
  TIM14_ARR = 19;
  TIM14_DIER |= 1;
  TIM14_CR1 = 1;

  enable_interrupts();
  NVIC_ISER = 1 << 19;

  /* Clear input string */

  for (int i = 0; i <= INPUT_MAX_LENGTH; i++)
    {
      input_string[i] = SPRITE_BLANK;
    }
}

void main()
{
  init();
  interface_draw_input();
  interface_draw_selection(SIDE_LEFT);
  interface_draw_selection(SIDE_RIGHT);
  interface_draw_battery();

  /* Configure sleep on ISR exit */

  SCR |= 1 << 1;

  while (1)
    {
      /* This shouldn't be needed, but it's here just in case */

      enter_sleep();
    }
}
