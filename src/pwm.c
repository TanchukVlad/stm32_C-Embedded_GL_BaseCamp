#include "pin.h"
#include "clock_168mhz.h"
#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#define PWM_CH1 1
#define PWM_CH2 2
#define PWM_CH3 3
#define PWM_CH4 4


const sk_pin led_green  = { .port=PORTD, .pin=12, .isinverse=false };
const sk_pin led_orange = { .port=PORTD, .pin=13, .isinverse=false };
const sk_pin led_red    = { .port=PORTD, .pin=14, .isinverse=false };
const sk_pin led_blue   = { .port=PORTD, .pin=15, .isinverse=false };


/* Store current PWM frequency */
static uint32_t current_timer_cnt_period;

void pwm_init(void)
{
        rcc_periph_clock_enable(RCC_TIM4);
        // The alignment and count direction.
        timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
        uint32_t tim_presс = (rcc_apb1_frequency * 2 / 1000000) - 1;
        timer_set_prescaler(TIM4, tim_presс);
        timer_enable_preload(TIM4);
        timer_continuous_mode(TIM4);
        // A timer update event is generated only after the specified number of repeat count cycles have been completed.
        timer_set_repetition_counter(TIM4, 0);
}

void led_init(void)
{
        rcc_periph_clock_enable(RCC_GPIOD);
        // GPIO mode alternative function
        gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE,
                        GPIO12 | GPIO13 | GPIO14 | GPIO15);

        // Set Push Pull and speed 50 MHz
        gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                		GPIO12 | GPIO13 | GPIO14 | GPIO15);
        //Set alternate function: TIM4 CH1/2/3/4
        gpio_set_af(GPIOD, GPIO_AF2, GPIO12 | GPIO13 | GPIO14 | GPIO15);

        timer_disable_oc_output(TIM4, TIM_OC1);
        timer_disable_oc_output(TIM4, TIM_OC2);
        timer_disable_oc_output(TIM4, TIM_OC3);
        timer_disable_oc_output(TIM4, TIM_OC4);
        //PWM mode 1
        timer_set_oc_mode(TIM4, TIM_OC1, TIM_OCM_PWM1);
        timer_set_oc_mode(TIM4, TIM_OC2, TIM_OCM_PWM1);
        timer_set_oc_mode(TIM4, TIM_OC3, TIM_OCM_PWM1);
        timer_set_oc_mode(TIM4, TIM_OC4, TIM_OCM_PWM1);
        // Enable preload register
        timer_enable_oc_preload(TIM4, TIM_OC1);
        timer_enable_oc_preload(TIM4, TIM_OC2);
        timer_enable_oc_preload(TIM4, TIM_OC3);
        timer_enable_oc_preload(TIM4, TIM_OC4);
        // Reset OC value
        timer_set_oc_value(TIM4, TIM_OC1, 0);
        timer_set_oc_value(TIM4, TIM_OC2, 0);
        timer_set_oc_value(TIM4, TIM_OC3, 0);
        timer_set_oc_value(TIM4, TIM_OC4, 0);
        // Enable OC output
        timer_enable_oc_output(TIM4, TIM_OC1);
        timer_enable_oc_output(TIM4, TIM_OC2);
        timer_enable_oc_output(TIM4, TIM_OC3);
        timer_enable_oc_output(TIM4, TIM_OC4);
}


void pwm_set_freq(uint32_t pwm_freq)
{
        current_timer_cnt_period = (rcc_apb1_frequency * 2 / (TIM4_PSC * pwm_freq));
        timer_set_period(TIM4, current_timer_cnt_period);

        timer_generate_event(TIM4, TIM_EGR_UG);
        timer_enable_counter(TIM4);
}


/* set DC value for a channel */
void pwm_set_dc(uint8_t ch_index, uint16_t dc_value_permillage)
{

        switch (ch_index) {
                case 1:
                        timer_set_oc_value(TIM4, TIM_OC1, dc_value_permillage);
        		break;
                case 2:
                        timer_set_oc_value(TIM4, TIM_OC2, dc_value_permillage);
                        break;
                case 3:
                        timer_set_oc_value(TIM4, TIM_OC3, dc_value_permillage);
                        break;
                case 4:
                        timer_set_oc_value(TIM4, TIM_OC4, dc_value_permillage);
                        break;
                default:
                        return;
        }
}


int main(void)
{
        clock_init();
        pwm_init();
        led_init();


        uint32_t period = 168000000ul / 10000ul;
        uint8_t priority = 2;
        sk_tick_init(period, priority);
        cm_enable_interrupts();

        pwm_set_freq(2000);   // 2000Hz

        int i = 0;

        while (1) {
                for (i; i < current_timer_cnt_period; i++) {
                        pwm_set_dc(PWM_CH1, i);
                        pwm_set_dc(PWM_CH2, i);
                        pwm_set_dc(PWM_CH3, i);
                        pwm_set_dc(PWM_CH4, i);
                        sk_tick_delay_ms(5);
                }
                for (i; i > 0; i--) {
                        pwm_set_dc(PWM_CH1, i);
                        pwm_set_dc(PWM_CH2, i);
                        pwm_set_dc(PWM_CH3, i);
                        pwm_set_dc(PWM_CH4, i);
                        sk_tick_delay_ms(5);
                }
        }
}
