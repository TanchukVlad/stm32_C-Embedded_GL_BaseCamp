#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>
#include <stdint.h>
#include <stddef.h>

/**
 * 168 MHz clock initialization.
 *
 * Context: Sets SYSCLK - 168 MHz, AHB - 168 MHz, APB1 - 42 MHz, APB2 - 84 MHz.
 */
void clock_init(void);
