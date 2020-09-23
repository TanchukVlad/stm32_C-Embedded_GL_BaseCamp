#include "pin.h"
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

const sk_pin led_green = { .port=PORTD, .pin=12, .isinverse=false};
const sk_pin led_orange = { .port=PORTD, .pin=13, .isinverse=false};
const sk_pin led_red = { .port=PORTD, .pin=14, .isinverse=false};
const sk_pin led_blue = { .port=PORTD, .pin=15, .isinverse=false};

const sk_pin swt1_btn = { .port=PORTC, .pin=11, .isinverse=true};
const sk_pin swt2_btn = { .port=PORTA, .pin=15, .isinverse=true};
const sk_pin swt3_btn = { .port=PORTC, .pin=9, .isinverse=true};
const sk_pin swt4_btn = { .port=PORTC, .pin=6, .isinverse=true};
const sk_pin swt5_btn = { .port=PORTC, .pin=8, .isinverse=true};


/**
 * Init func fot exti.
 */
void sk_inter_exti_init(sk_pin pin, enum exti_trigger_type trig)
{
	exti_select_source((1 << pin.pin), sk_pin_port_to_gpio(pin.port));
	exti_set_trigger((1 << pin.pin), trig);
	exti_enable_request((1 << pin.pin));
	exti_reset_request((1 << pin.pin));
}

void mode_setup(sk_pin pin, uint8_t mode, uint8_t pull_up_down)
{
	gpio_mode_setup(sk_pin_port_to_gpio(pin.port), mode,
					    pull_up_down , (1 << pin.pin));
}


void softdelay(uint32_t N)
{
	while (N--) __asm__("nop");
}


void all_led_set(bool mode)
{
	sk_pin_set(led_green, mode);
	sk_pin_set(led_orange, mode);
	sk_pin_set(led_red, mode);
	sk_pin_set(led_blue, mode);
}

//uint16_t pressed_cnt = 0;
//uint16_t released_cnt = 0;

bool btn_debounce(sk_pin btn)
{

        uint16_t pressed_cnt = 0;

	for (int i = 0; i < 1000; i++) {
		if (sk_pin_read(btn))
                        pressed_cnt++;
		if (pressed_cnt > 300) {
			while (!sk_pin_read(btn));
			return true;
		}
	}
	return false;
}


/**
 * swt1_btn->led_blue
 * swt2_btn->led_off
 * swt3_btn->led_orange
 * swt4_btn->led_red
 * swt5_btn->led_green
 */
void exti15_10_isr(void)
{
	if (exti_get_flag_status(EXTI11)) {
                if (btn_debounce(swt1_btn))
                        sk_pin_toggle(led_blue);
                exti_reset_request((1 << swt1_btn.pin));
        }
        if (exti_get_flag_status(EXTI15)) {
                if (btn_debounce(swt2_btn)) {
			all_led_set(false);
                }
                exti_reset_request((1 << swt2_btn.pin));
        }
}


void exti9_5_isr(void)
{
	if (exti_get_flag_status(EXTI9)) {

                if (btn_debounce(swt3_btn))
                        sk_pin_toggle(led_orange);
                exti_reset_request((1 << swt3_btn.pin));
        }
	if (exti_get_flag_status(EXTI6)) {
                if (btn_debounce(swt4_btn))
                        sk_pin_toggle(led_red);
                exti_reset_request((1 << swt4_btn.pin));
        }
	if (exti_get_flag_status(EXTI8)) {
                if (btn_debounce(swt5_btn))
                        sk_pin_toggle(led_green);
                exti_reset_request((1 << swt5_btn.pin));
        }
}


int main(void)
{
        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOC);
        // Enable EXTI port clock
	// This is not obvious, but selected port is stored in syscfg registers
	rcc_periph_clock_enable(RCC_SYSCFG);

        sk_pin_mode_setup(led_green, MODE_OUTPUT);
        sk_pin_mode_setup(led_orange, MODE_OUTPUT);
        sk_pin_mode_setup(led_red, MODE_OUTPUT);
        sk_pin_mode_setup(led_blue, MODE_OUTPUT);
	//leds on
	all_led_set(true);

	mode_setup(swt1_btn, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP);
	mode_setup(swt2_btn, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP);
	mode_setup(swt3_btn, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP);
	mode_setup(swt4_btn, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP);
	mode_setup(swt5_btn, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP);

        //Set priority group. Programing manual p.228.
        scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP4_SUB4);
        uint8_t group = 0;
        uint8_t subgroup = 1;
        nvic_set_priority(NVIC_EXTI9_5_IRQ, (group << 2) | subgroup);
	nvic_set_priority(NVIC_EXTI15_10_IRQ, ((1 + group) << 2) | subgroup);

        sk_inter_exti_init(swt1_btn, EXTI_TRIGGER_RISING);
	sk_inter_exti_init(swt2_btn, EXTI_TRIGGER_RISING);
	sk_inter_exti_init(swt3_btn, EXTI_TRIGGER_RISING);
	sk_inter_exti_init(swt4_btn, EXTI_TRIGGER_RISING);
	sk_inter_exti_init(swt5_btn, EXTI_TRIGGER_RISING);

        //Enable interrupt (IRQ);
        nvic_enable_irq(NVIC_EXTI9_5_IRQ);
	nvic_enable_irq(NVIC_EXTI15_10_IRQ);
        // Globally enable interrupts. Just to show how it's done
        cm_enable_interrupts();

	//leds off
	all_led_set(false);


        while(1) {

        }
}
