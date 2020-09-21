#include "lcd_hd44780.h"
#include "freq_read.h"
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include <stddef.h>


uint8_t color_scale(uint32_t freq, char color);


void color_get_rgb(sk_pin_group group, uint32_t ms, uint32_t *rgb_arr);


uint8_t color_name(uint32_t *rgb_arr);
