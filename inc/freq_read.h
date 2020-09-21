#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

void timer3_init(void);


void timer2_init(void);


void timer5_init(void);


uint32_t freq_read(uint32_t ms);
