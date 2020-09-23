#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * TIM3 initialization.
 *
 * Context: TIM3 install like a master timer and he is counting time in which
 *          slave timer is counting signals on the ETRF. TIM3 starts counting
 *          in the same time with slave timer. In the end of counting makes
 *          interrupt and count in the interrupt service routines number of
 *          impulse by time.
 */
void freq_read_timer3_init(void);

/**
 * TIM2 initialization.
 *
 * Context: TIM2 install like a slave timer. Clock sourse: External clock mode 2.
 *          The counter is clocked by any active edge on the ETRF signal.
 *          TIM2 starts counting in the same time with master timer.
 *          Use Gated Mode - The counter clock is enabled when the trigger
 *          input (TRGI) is high. TIM2 has max period. TIM2 is master timer for
 *          third timer which counts num of owerflow  of the TIM2.
 */
void freq_read_timer2_init(void);

/**
 * TIM5 initialization.
 *
 * Context: TIM5 install like a slave timer. Slave trigger source: External Trigger
 *          input (ETRF). Counts num of owerflow of the master timer.
 */
void freq_read_timer5_init(void);

/**
 * Read frequency.
 * @ms: Time of measuring in ms.
 *
 * Context: TIM3, TIM2 and TIM5 start counting. When they are counting,
 *          system wait for interrupt.
 *
 * Return: Frequency in KHz.
 */
uint32_t freq_read(uint32_t ms);
