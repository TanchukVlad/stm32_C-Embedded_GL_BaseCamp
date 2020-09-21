#include "clock_168mhz.h"
#include "delay.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define SERVO_CH1 1
#define SERVO_CH2 2


void pwm_init(void);


void servo_init(void);


void pwm_set_freq(uint32_t pwm_freq);


void pwm_set_dc(uint8_t ch_index, uint16_t dc_value_permillage);


void pwm_set_servo(uint8_t ch_index, uint8_t deg);


void servo_choose_cell(uint8_t ch_index, uint8_t cell);


void servo_start(uint8_t ch_index);
