#include "pin.h"
#include "tick.h"
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/pwr.h>

const sk_pin led_green = { .port=PORTD, .pin=12, .isinverse=false};
const sk_pin led_blue = { .port=PORTD, .pin=15, .isinverse=false};
const sk_pin us_btn = { .port=PORTA, .pin=0, .isinverse=false};
const sk_pin swt1_btn = { .port=PORTC, .pin=11, .isinverse=true};

void softdelay(uint32_t N)
{
	while (N--) {
                __NOP();
        }
}


void sk_inter_exti_init(sk_pin pin, enum exti_trigger_type trig)
{
	exti_select_source((1 << pin.pin), sk_pin_port_to_gpio(pin.port));
	exti_set_trigger((1 << pin.pin), trig);
	exti_enable_request((1 << pin.pin));
	exti_reset_request((1 << pin.pin));
}



void pause_inter_init(uint16_t nvic_exti, uint8_t group, uint8_t subgroup)
{
        rcc_periph_clock_enable(RCC_SYSCFG);
        scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP4_SUB4);
	nvic_set_priority(nvic_exti, (group << 2) | subgroup);

        sk_inter_exti_init(swt1_btn, EXTI_TRIGGER_FALLING);

        nvic_enable_irq(nvic_exti);
}

uint8_t mode = 0;

void pause(void)
{
        if (mode == 1) {
                //sk_lcd_set_addr(&lcd, 0x00);
                //lcd_clear_display(&lcd);
                //lcd_print_text(&lcd, "Pause!");
                //pwr_set_standby_mode();
                __WFI();
        } else if (mode > 1) {
                //pwr_set_stop_mode();
                //lcd_print_text(&lcd, "Starting...");
                mode = 0;
        }
}


void exti15_10_isr(void)
{
	softdelay(2000);
	if (!sk_pin_read(swt1_btn)) {
		sk_pin_toggle(led_green);
                sk_pin_toggle(led_blue);
	}
        exti_reset_request((1 << swt1_btn.pin));
}


int main(void)
{
        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_GPIOD);

        sk_pin_mode_setup(led_green, MODE_OUTPUT);
        sk_pin_mode_setup(led_blue, MODE_OUTPUT);
        sk_pin_mode_setup(us_btn, MODE_INPUT);
        sk_pin_mode_setup(swt1_btn, MODE_INPUT);

        pause_inter_init(NVIC_EXTI15_10_IRQ, 0, 1);

        uint32_t period = 16000000ul / 10000ul;
        uint8_t priority = 2;
        sk_tick_init(period, priority);

        sk_pin_set(led_green, true);  //led on

        while(1) {
                sk_pin_toggle(led_green);
                sk_tick_delay_ms(500);
                //pause();
        }
}
