#include "tick.h"
#include "lcd_hd44780.h"
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include <stddef.h>


uint32_t read_freq(sk_pin pin, uint32_t ms);


uint8_t color_scale(uint32_t freq, char color);


void color_get_rgb(sk_pin_group group, sk_pin pin, uint32_t ms, uint8_t *rgb_arr);


char color_name(uint8_t *rgb_arr);
