#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>


void timer3_init(void);


void timer2_init(void);


void freq_read_start(uint16_t sec);


uint32_t freq_read(void);
