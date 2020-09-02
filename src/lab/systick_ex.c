#include "pin.h"
#include "macro.h"
#include "tick.h"
#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>

const sk_pin led_red = { .port=PORTD, .pin=14, .isinverse=false};

int main(void) {

        rcc_periph_clock_enable(RCC_GPIOD);

        sk_pin_mode_setup(led_red, MODE_OUTPUT);

        sk_pin_set(led_red, true);

        uint32_t period = 1000;
        uint8_t priority = 2;
        sk_tick_init(period, priority);
        cm_enable_interrupts();

        while (1) {
                sk_tick_delay_ms(500);
                sk_pin_toggle(led_red);
        }
}
