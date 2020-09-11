#include "clock_168mhz.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define PWM_CH1 1
#define PWM_CH2 2


void pwm_init(void);


void servo_init(void);


void pwm_set_freq(uint32_t pwm_freq);


void pwm_set_dc(uint8_t ch_index, uint16_t dc_value_permillage);


void pwm_set_servo(uint8_t ch_index, uint8_t deg);
