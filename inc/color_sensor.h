#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include <stddef.h>

typedef struct sk_pin_group sk_pin_group;

/**
 * Color sensor get RGB.
 * @group: struct of group of pin.
 * @ms: Time in millisecond.
 * @*rgb_arr: Array in which will be written R, G and B components.
 *
 * Context: Reads frequency from sensor for each component(R, G and B) and writes
 *          these value in rgb_array.
 */
void color_get_rgb(sk_pin_group group, uint32_t ms, uint32_t *rgb_arr);

/**
 * Color sensor get name of color.
 * @*rgb_arr: with R, G and B components.
 *
 * Context: Compares value from rgb_arr and returns number of cell in which
 *          will be thrown candy depends on color.
 *
 * Return: Number of cell depends on color.
 */
uint8_t color_name(uint32_t *rgb_arr);
