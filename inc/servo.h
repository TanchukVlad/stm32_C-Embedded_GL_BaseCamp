#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define SERVO_CH1 1
#define SERVO_CH2 2

/**
 * Pulse width modulation initialization.
 *
 * Context: Initializes TIM4 which used for pwm mode and sets timer prescaler.
 */
void servo_pwm_init(void);

/**
 * Servo initialization.
 *
 * Context: Initializes pins which is connected to the servos. And continue pwm sets up.
 */
void servo_init(void);

/**
 * Set pwm frequency.
 * @pwm_freq: Pwm frequency in Hz.
 *
 * Context: Set pwm frequency in Hz.
 */
void servo_pwm_set_freq(uint32_t pwm_freq);

/**
 * Set PWM duty cycle.
 * @ch_index: OC channel designators TIM_OC1 or TIM_OC2.
 * @dc_value_permillage: Duty cycle value in ms.
 *
 * Context: Set PWM duty cycle in ms.
 */
void servo_pwm_set_dc(uint8_t ch_index, uint16_t dc_value_permillage);

/**
 * Set servo position.
 * @ch_index: OC channel designators TIM_OC1 or TIM_OC2.
 * @deg: Angle in degrees.
 *
 * Context: Set servo position on the angle in degrees from 0 to 180 degrees.
 */
void servo_pwm_set_servo(uint8_t ch_index, uint8_t deg);

/**
 * Choose cell for candy.
 * @ch_index: OC channel designators TIM_OC1 or TIM_OC2.
 * @cell: Number of cell.
 *
 * Context: Choose cell for candy depending on color of it.
 */
void servo_choose_cell(uint8_t ch_index, uint8_t cell);

/**
 * Starts servo calibration.
 * @ch_index: OC channel designators TIM_OC1 or TIM_OC2.
 *
 * Context: Using for checking servo position.
 */
void servo_start(uint8_t ch_index);
