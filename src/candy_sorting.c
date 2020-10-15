/**
 * -------------------------------CANDY SORTING----------------------------
 * This program sorts candies in the correct cells depends on color of candy.
 * For realisation this device, was used:
 *      Servo Motor SG90 - 2 pieces
 *      Color Sensor TCS3200 - 1 pieces
 *      GL Starter Kit with stm32f407VG - Discovery
 * Color is read by Color Sensor which returns some frequency for each
 * component(R, G and B).
 * Execution order:
 *      1) Servo 1 directs the candy on the Color Sensor;
 *      2) The Color Sensor recognize color;
 *      3) Servo 2 sets slide on the correct cell depends on color;
 *      4) Servo 1 sends candy on the slide.
 * Program starts with calibration. You can set your cells in the correct place.
 * If you were late, you can repeat calibration.
 * During the program, you are able to make pause until you press button.
 */

#include "clock_168mhz.h"
#include "lcd_hd44780.h"
#include "delay.h"
#include "freq_read.h"
#include "color_sensor.h"
#include "servo.h"
#include "printf.h"
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>

/** Three position for servo 1. */
#define S1_ANGLE_1 180
#define S1_ANGLE_2 90
#define S1_ANGLE_3 5

#define POS1_DELAY 400
#define POS2_DELAY 10
#define POS3_DELAY 200

/** Control buttons. */
sk_pin btn_1 = { .port = PORTC, .pin = 11, .isinverse = true };
sk_pin btn_2 = { .port = PORTA, .pin = 15, .isinverse = true };
sk_pin btn_3 = { .port = PORTC, .pin = 9, .isinverse = true };

/** Rs, rw, en, bkl pins for lcd. */
sk_pin lcd_rs = { .port = PORTE, .pin = 7, .isinverse = false };
sk_pin lcd_rw = { .port = PORTE, .pin = 10, .isinverse = false };
sk_pin lcd_en = { .port = PORTE, .pin = 11, .isinverse = false };
sk_pin lcd_bkl = { .port = PORTE, .pin = 9, .isinverse = false };

/** LCD group for 4-Bit Ineterface: PE15, PE14, PE13, PE12. */
sk_pin_group lcd_group = {
        .port = PORTE,
        .pins = (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12),
        .inversions = false
};


struct sk_lcd lcd = {
        .pin_group_data = &lcd_group,
        .pin_rs = &lcd_rs,
        .pin_rw = &lcd_rw,
        .pin_en = &lcd_en,
        .pin_bkl = &lcd_bkl,
        .delay_func_us = NULL,
        .delay_func_ms = &delay_ms,
        .is4bitinterface = 1
};

/** cs_out PA5 - Output frequency from color sensor.*/
sk_pin cs_out = { .port = PORTA, .pin = 5, .isinverse = false };
sk_pin_group cs_s_group = {
        .port = PORTD,
        .pins = (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3),
        .inversions = false
};


bool confirm_btn = true;
bool pause_mode = false;
// Red, Green, Yellow, Orange, Violet, Unknown
uint16_t count_arr[] = {0, 0, 0, 0, 0, 0};

/**
 * Print counter array.
 *
 * Context: Print the current count of candy on the LCD. Using snprintf().
 */
void print_count_arr(void)
{
        char buf[32];
        snprintf(buf, sk_arr_len(buf), "R=%u G=%u Y=%u\nO=%u V=%u U=%u",
                count_arr[0], count_arr[1], count_arr[2], count_arr[3],
                count_arr[4], count_arr[5]);
        sk_lcd_set_addr(&lcd, 0x00);
        lcd_clear_display(&lcd);
        lcd_print_text(&lcd, buf);
}


/**
 * Eternal interrupt initialization.
 * @pin: Struct of pin.
 * @trig: External interrupt trigger type.
 *
 * Context: Sets external interrupt source, trigger type, enables and reset
 *          request for pin.
 */
void exti_init(struct sk_pin pin, enum exti_trigger_type trig)
{
	exti_select_source((1 << pin.pin), sk_pin_port_to_gpio(pin.port));
	exti_set_trigger((1 << pin.pin), trig);
	exti_enable_request((1 << pin.pin));
	exti_reset_request((1 << pin.pin));
}

/**
 * Pause.
 *
 * Context: Makes program's pause. Fires when pressed btn_2. Repeated pressing
 *          turn off pause. Prints on the lcd "Pause!" and the current count of
 *          candy using print_count_arr(). During the pause is waiting for interrupt.
 */
void pause(void)
{
        if (pause_mode) {
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_clear_display(&lcd);
                lcd_print_text(&lcd, "     Pause!     \n    press 2     ");
                delay_ms(3000);
                print_count_arr();
                pwr_set_standby_mode();
                __WFI();
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_clear_display(&lcd);
                pwr_set_stop_mode();
                lcd_print_text(&lcd, "  Continue...   ");
                pause_mode = false;
        }
}

/**
 * Control buttons initialization.
 *
 * Context: Initializes control buttons, sets nvic priority and enable irq.
 */
void btn_init(void)
{
        rcc_periph_clock_enable(RCC_SYSCFG);

        sk_pin_mode_setup(btn_1, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP);
        sk_pin_mode_setup(btn_2, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP);
        sk_pin_mode_setup(btn_3, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP);

        scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP4_SUB4);
        nvic_set_priority(NVIC_EXTI9_5_IRQ, ((0 << 2 | 1) << 4));
	nvic_set_priority(NVIC_EXTI15_10_IRQ, ((1 << 2 | 1) << 4));

        exti_init(btn_1, EXTI_TRIGGER_RISING);
        exti_init(btn_2, EXTI_TRIGGER_RISING);
        exti_init(btn_3, EXTI_TRIGGER_RISING);

        nvic_enable_irq(NVIC_EXTI9_5_IRQ);
	nvic_enable_irq(NVIC_EXTI15_10_IRQ);
}

/**
 * Soreing calibration.
 *
 * Context: Sets servos in all positions and gives a opportunity to place cells
 *          on the correct place.
 */
void sorting_calibration(void)
{
        bool semaphore = true;

        while(semaphore) {
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_print_text(&lcd, "    Starting    \n  calibration!  ");
                servo_start(SERVO_CH1);
                servo_start(SERVO_CH2);
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_clear_display(&lcd);
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_print_text(&lcd, "YES  Repeat   NO\n3 calibration? 1");
                __WFI();
                if (confirm_btn) {
                        continue;
                } else {
                        semaphore = false;
                        lcd_clear_display(&lcd);
                        sk_lcd_set_addr(&lcd, 0x00);
                        lcd_print_text(&lcd, "Sorting started");
                        break;
                }
        }
}


/**
 * Interrupt service routine for btn_1 and btn_2.
 *
 * Context: EXTI11 and EXTI15 handled by exti15_10_isr.
 */
void exti15_10_isr(void)
{
        if (exti_get_flag_status(EXTI11)) {
                confirm_btn = false;
                exti_reset_request((1 << btn_1.pin));
        }
        if (exti_get_flag_status(EXTI15)) {
                pause_mode = !pause_mode;
                exti_reset_request((1 << btn_2.pin));
        }
}

/**
 * Interrupt service routine for btn_3.
 *
 * Context: EXTI9 handled by exti9_5_isr.
 */
void exti9_5_isr(void)
{
        if (exti_get_flag_status(EXTI9)) {
                confirm_btn = true;
                exti_reset_request((1 << btn_3.pin));
        }
}


int main(void)
{
        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_GPIOC);
        rcc_periph_clock_enable(RCC_GPIOD);
        rcc_periph_clock_enable(RCC_GPIOE);
        // LCD pins init
        sk_pin_group_mode_setup(lcd_group, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_rs, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_rw, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_en, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_bkl, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        // Color Sensor pins init
        sk_pin_group_mode_setup(cs_s_group, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(cs_out, GPIO_MODE_AF, GPIO_PUPD_NONE);
        gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO5);
        gpio_set_af(GPIOA, GPIO_AF1, GPIO5);

        clock_init();
        servo_pwm_init();
        servo_init();
        delay_timer_init();
        freq_read_timer3_init();
        freq_read_timer2_init();
        freq_read_timer5_init();
        btn_init();

        cm_enable_interrupts();

        //LCD init
        sk_pin_group_set(lcd_group, 0x00);
        lcd_init_4bit(&lcd);
        sk_lcd_set_backlight(&lcd, true);

        lcd_clear_display(&lcd);
        lcd_print_text(&lcd, "Candy sorting!");
        delay_ms(1000);

        servo_pwm_set_freq(50);   // Set 50Hz for PWM

        sorting_calibration();

        while (1) {
                // Take a candy
                servo_pwm_set_servo(SERVO_CH1, S1_ANGLE_1);
                delay_ms(POS1_DELAY);
                // If pause_mode = true, will be pause
                pause();
                // Recognize color for the candy
                servo_pwm_set_servo(SERVO_CH1, S1_ANGLE_2);
                uint32_t rgb[3];
                color_get_rgb(cs_s_group, 400, rgb);
                uint8_t cell = color_name(rgb);
                // Set slide (serco 2) in the correct position
                servo_choose_cell(SERVO_CH2, cell);
                // Increment candy count
                count_arr[cell]++;
                delay_ms(POS2_DELAY);
                // Send candy on the slide
                servo_pwm_set_servo(SERVO_CH1, S1_ANGLE_3);
                print_count_arr();
                delay_ms(POS3_DELAY);
        }
}
