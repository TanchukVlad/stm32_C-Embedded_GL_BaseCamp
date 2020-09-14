#include "servo.h"


static uint32_t current_timer_cnt_period;


void pwm_init(void)
{
        rcc_periph_clock_enable(RCC_TIM3);
        // The alignment and count direction.
        timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
        uint32_t tim_presс = (rcc_apb1_frequency * 2 / 1000000) - 1;
        timer_set_prescaler(TIM3, tim_presс);
        timer_enable_preload(TIM3);
        timer_continuous_mode(TIM3);
        // A timer update event is generated only after the specified number of repeat count cycles have been completed.
        timer_set_repetition_counter(TIM3, 0);
}


void servo_init(void)
{
        rcc_periph_clock_enable(RCC_GPIOC);
        // GPIO mode alternative function
        gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);

        // Set Push Pull and speed 50 MHz
        gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                		GPIO6 | GPIO7);
        //Set alternate function: TIM3 CH1/2/3/4
        gpio_set_af(GPIOC, GPIO_AF2, GPIO6 | GPIO7);

        timer_disable_oc_output(TIM3, TIM_OC1);
        timer_disable_oc_output(TIM3, TIM_OC2);
        //PWM mode 1
        timer_set_oc_mode(TIM3, TIM_OC1, TIM_OCM_PWM1);
        timer_set_oc_mode(TIM3, TIM_OC2, TIM_OCM_PWM1);

        // Enable preload register
        timer_enable_oc_preload(TIM3, TIM_OC1);
        timer_enable_oc_preload(TIM3, TIM_OC2);

        // Reset OC value
        timer_set_oc_value(TIM3, TIM_OC1, 0);
        timer_set_oc_value(TIM3, TIM_OC2, 0);

        // Enable OC output
        timer_enable_oc_output(TIM3, TIM_OC1);
        timer_enable_oc_output(TIM3, TIM_OC2);
}


void pwm_set_freq(uint32_t pwm_freq)
{
        current_timer_cnt_period = (rcc_apb1_frequency * 2 / (TIM3_PSC * pwm_freq));
        timer_set_period(TIM3, current_timer_cnt_period);

        timer_generate_event(TIM3, TIM_EGR_UG);
        timer_enable_counter(TIM3);
}


/* set DC value for a channel */
void pwm_set_dc(uint8_t ch_index, uint16_t dc_value_permillage)
{

        switch (ch_index) {
                case 1:
                        timer_set_oc_value(TIM3, TIM_OC1, dc_value_permillage);
        		break;
                case 2:
                        timer_set_oc_value(TIM3, TIM_OC2, dc_value_permillage);
                        break;
                default:
                        return;
        }
}


void pwm_set_servo(uint8_t ch_index, uint8_t deg)
{
        uint16_t dc_val = 11 * deg + 380;  //convert deg
        pwm_set_dc(ch_index, dc_val);
}


void servo_choose_cell(uint8_t ch_index, uint8_t cell)
{
        switch (cell) {
                case 0: // Red
                        pwm_set_servo(ch_index, 0);
                        break;
                case 1: // Green
                        pwm_set_servo(ch_index, 36);
                        break;
                case 2: // Yellow
                        pwm_set_servo(ch_index, 72);
                        break;
                case 3: // Orange
                        pwm_set_servo(ch_index, 108);
                        break;
                case 4: // Violet
                        pwm_set_servo(ch_index, 144);
                        break;
                default:
                        pwm_set_servo(ch_index, 180);
                        break;
                }
}


void servo_start(uint8_t ch_index)
{
        for (int i = 0; i < 6; i++)
        {
                servo_choose_cell(ch_index, i);
                sk_tick_delay_ms(3000);
        }
}
