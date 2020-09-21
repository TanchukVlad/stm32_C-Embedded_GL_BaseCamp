#include "servo.h"
//#include "tick.h"
#include "lcd_hd44780.h"
#include "printf.h"


int main(void)
{
        rcc_periph_clock_enable(RCC_GPIOD);

        clock_init();
        pwm_init();
        servo_init();
        delay_timer_init();

        /*uint32_t period = 168000000ul / 10000ul;
        uint8_t priority = 2;
        sk_tick_init(period, priority);*/
        cm_enable_interrupts();

        pwm_set_freq(50);   // 50Hz

        //uint16_t deg = 0;    // 0deg
        uint8_t cell = 0;
        while (1) {
                servo_choose_cell(SERVO_CH2, cell);
                delay_ms(1000);
                cell++;
                if (cell > 5)
                        cell = 0;
        }
}
