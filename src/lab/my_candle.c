#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdint.h>

/**
 * Imitation of the candle using softdelay and my randomizer
 *
 * Note:
 * Randomizer was build using a linear congruential generator (LCG)   
 * LCG - is an algorithm that yields a sequence of pseudo-randomized 
 * numbers calculated with a discontinuous piecewise linear equation.
 * https://en.wikipedia.org/wiki/Linear_congruential_generator
 */
void softdelay(volatile uint32_t N)
{
    while (N--);
}

int my_rand(int x0)
{
    int a = 106;
    int c = 1283;
    int m = 6075;
    
    return (a * x0 + c) % m;    
}

void pwm_duty(int a)
{
    gpio_clear(GPIOD, GPIO12);
    softdelay(a*100);
    gpio_set(GPIOD, GPIO12);
    softdelay((15-a)*100);
}

int main(void)
{
    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12);
    gpio_set(GPIOD, GPIO12);
    int r = my_rand(0);
    while (1) {
        if (r > 3037)
        {
            pwm_duty(15);  // maximum pwm level
        } else if (r > 2976) {
            pwm_duty(14);
        } else if (r > 2679) {
            pwm_duty(13);
        } else if (r > 2381) {
            pwm_duty(12);
        } else if (r > 2083) {
            pwm_duty(11);
        } else if (r > 1786) {
            pwm_duty(10);
        } else if (r > 1488) {
            pwm_duty(9);
        } else if (r > 1190) {
            pwm_duty(8);
        } else if (r > 893) {
            pwm_duty(7);
        } else if (r > 595) {
            pwm_duty(6);
        } else if (r > 297) {
            pwm_duty(5);
        } else if (r > 60) {
            pwm_duty(4);
        } else if (r > 45) {
            pwm_duty(3);
        } else if (r > 30) {
            pwm_duty(2);
        } else if (r > 15) {
            pwm_duty(1);
        } else {
            pwm_duty(0);  // logical 0
        }
        r = my_rand(r);     
    }
}
