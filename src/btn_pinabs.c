#include "pin.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdint.h>
#include <stdbool.h>

const sk_pin led_green = { .port=PORTD, .pin=12, .isinverse=false};
const sk_pin btn_up = { .port=PORTC, .pin=9, .isinverse=true};
const sk_pin btn_down = { .port=PORTC, .pin=11, .isinverse=true};

void softdelay(volatile uint32_t N)
{
	while (N--);
}

void pwm_duty(int a)
{
	sk_pin_set(led_green, false);
	softdelay(a*1000);
	sk_pin_set(led_green, true);
	softdelay((30-a)*1000);
}


int main(void)
{
	rcc_periph_clock_enable(RCC_GPIOD);	// leds are here
	rcc_periph_clock_enable(RCC_GPIOC);	// glsk buttons are here
	sk_pin_mode_setup(led_green, MODE_OUTPUT);
	sk_pin_mode_setup(btn_up, MODE_INPUT);
	sk_pin_mode_setup(btn_down, MODE_INPUT);

	sk_pin_set(led_green, true);
	
	uint8_t sp = 29;
	
	while (1) {
		pwm_duty(sp);

		if (sk_pin_read(btn_up)) {	// button up (SWT3)
			sp++;
			if (sp > 30)
				sp = 1;
		}
		
		if (sk_pin_read(btn_down)) {	// button down (SWT1)
			sp--;
			if (sp < 1)
                		sp = 30;
		}
	}
}
