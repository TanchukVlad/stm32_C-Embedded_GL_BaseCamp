/**
 * This example demonstrates use of semihosting feature.
 *
 * Semihosting allows to output information such as generic stdout or debug log
 * from MCU to PC via debugger.
 * The end result is: we use regular printf() and see its output in terminal
 * as if the program was running on the PC.
 *
 * Semihosting puts data in MCU buffer, then traps MCU and waits for debugger
 * to respond. Debugger communicates with OpenOCD, which reads data from buffer
 * in a way similar to reading variable values during gdb session. And then
 * returns control to MCU.
 * Semihosting does not require any specific hardware features. But it is slow.
 * Usually slower than SWO ITM as SWO could output data directly on single pin
 * at the same time when MCU is busy doing some real work, without need to stop
 * and trap MCU.
 *
 * Semihosting is very useful for debugging. But the downside is that the firmware
 * will not work standalone without the debugger being connected.
 * We can overcome this by introducing two build profiles: debug and release
 *
 * HOW TO USE SEMIHOSTING:
 * 1. Link with `--specs=rdimon.specs -lrdimon` and DO NOT use `-lnosys`
 * 2. In OpenOCD use command `arm semihosting enable` before initialise_monitor_handles
 *    is first called by your program. If you are using openocd directly, that's enough.
 *
 *    If you are using OpenOCD run via GDB pipe, in GDB session run:
 *        monitor arm semihosting enable
 *        monitor arm semihosting_fileio enable
 *    and you'll probably need to reset MCU, in GDB session:
 *        monitor reset halt
 * 3. Don't forget to use `initialise_monitor_handles` in your program code
 */

 #include "pin.h"
 #include "tick.h"
 #include <libopencm3/cm3/cortex.h>
 #include <libopencm3/cm3/nvic.h>
 #include <libopencm3/stm32/adc.h>
 #include <libopencm3/stm32/rcc.h>
 #include <libopencm3/stm32/gpio.h>
 #include <stdio.h>
 #include <stdint.h>
 #include <stddef.h>

// USE_SEMIHOSTING is defined via our Makefile
#if defined(USE_SEMIHOSTING) && USE_SEMIHOSTING
// Usually we try not to use standard C library. But in this case we need printf
#include <stdio.h>

// this is our magic from librdimon
// should be called in main before first output
extern void initialise_monitor_handles(void);
#else
// this is how semihosting may be used conditionally
#error "This example requires SEMIHOSTING=1"
#endif


const sk_pin temp_pin = { .port=PORTB, .pin=1, .isinverse=false};


static void adc_temp_init(void)
{
        rcc_periph_clock_enable(RCC_GPIOB);
        //minimize noise
        gpio_set_output_options(temp_pin.port, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, temp_pin.pin);
        //gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 1 << 1);
        sk_pin_mode_setup(temp_pin, MODE_ANALOG);
        // APB2 prescaler /2
        rcc_set_ppre2(RCC_CFGR_PPRE_DIV_2);
        // ADC1 clock
        rcc_periph_clock_enable(RCC_ADC1);
        // ADC1 prescaler /8
        adc_set_clk_prescale(ADC_CCR_ADCPRE_BY8);

        // Set ADC resolution to 12 bit
        adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
        // Set left-to-right data alignment in adc result register
        adc_set_right_aligned(ADC1);

        // Set ADC regular sequence register;
        uint8_t channels[16];
        for (uint8_t i = 0; i < sk_arr_len(channels); i++) {
                // Set sampling time:
                // Set ADC sample time register: 480 cycles.
                // Fs = fadc / 480 = 1 MHz / 480 = 2083.33 Hz.
                adc_set_sample_time(ADC1, i, ADC_SMPR_SMP_480CYC);
                channels[i] = 9;
        }
        adc_set_regular_sequence(ADC1, sk_arr_len(channels), channels);
        // The EOC is set after each conversion in a sequence rather than at the end of the sequence.
        adc_eoc_after_each(ADC1);

        adc_enable_scan_mode(ADC1);
        //In this mode the ADC performs a conversion of one channel or a channel group and stops.
        adc_set_single_conversion_mode(ADC1);

        adc_enable_eoc_interrupt(ADC1);
        //The overrun interrupt is generated when data is not read from a result register before the next conversion is written.
        //If DMA is enabled, all transfers are terminated and any conversion sequence is aborted.
        adc_enable_overrun_interrupt(ADC1);

        nvic_set_priority(NVIC_ADC_IRQ, 10);
        nvic_enable_irq(NVIC_ADC_IRQ);

        adc_power_on(ADC1);
        sk_tick_delay_ms(10);
}

static volatile uint16_t __adc_avgval = 0;

void adc_isr(void)
{
        static volatile uint32_t sum = 0;
        static volatile uint32_t cnt = 0;


        if (adc_get_overrun_flag(ADC1)) {
                sum = cnt = 0;
                adc_clear_flag(ADC1, ADC_SR_OVR);
                return;
        }

        sum += adc_read_regular(ADC1) & 0x00000FFF;
        cnt++;
        if (cnt >= 16) {
                __adc_avgval = sum / cnt;
                printf("--------------------adc_isr(if_start)------------------\n");
                printf("cnt = %ld\n", cnt);
                printf("sum = %ld\n", sum);
                printf("__adc_avgval = %d\n", __adc_avgval);
                sum = cnt = 0;
                adc_clear_flag(ADC1, ADC_SR_STRT);
                printf("--------------------adc_isr(if_end)--------------------\n");
        }
        adc_clear_flag(ADC1, ADC_SR_EOC);

}


static uint16_t adc_acquire(void)
{
        //This starts conversion on a set of defined regular channels if the ADC trigger is set to be a software trigger.
        //It is cleared by hardware once conversion starts.
        adc_start_conversion_regular(ADC1);

        while (adc_get_flag(ADC1, ADC_SR_STRT)) {
                __WFI();
        }
        return __adc_avgval;  //converted value after averaging in ISR
}



static inline float adc_temp_convert(uint16_t adcval)
{
        return 97.31 - 0.04039 * adcval;
}


int main(void)
{
	initialise_monitor_handles();
	printf("Monitor initialized. WE GET IT THROUGH SEMIHOSTING\n");

        uint32_t period = 16000000ul / 10000ul;
        uint8_t priority = 2;
        sk_tick_init(period, priority);
        cm_enable_interrupts();

        adc_temp_init();

	printf("System initialized\n");
        while (1) {
                uint16_t adcval = adc_acquire();
                float temp = adc_temp_convert(adcval);
                printf("adcval = %d\n", adcval);
                printf("temp = %5.1f\n", temp);
                /*char tmp[20], buffer[20];
		snprintf(tmp, sk_arr_len(tmp), "%5.1f", temp);
		snprintf(buffer, sk_arr_len(buffer), "T=%-5sC A=%u", tmp, (unsigned int)adcval);
		lcd_print_text(&lcd, buffer);*/
        }
}
