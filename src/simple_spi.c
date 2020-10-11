#include "lcd_hd44780.h"
#include "pin.h"
#include "delay.h"
#include "macro.h"
#include "clock_168mhz.h"
#include "printf.h"
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <stdint.h>
#include <stddef.h>


sk_pin led_green = {.port = PORTD, .pin = 12, .isinverse = false};

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


sk_pin spiflash_ce = { .port = PORTD, .pin = 7, .isinverse = false };


void spi_init(void)
{
        // Setup GPIO
        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_GPIOB);
        rcc_periph_clock_enable(RCC_GPIOD);

        // Pins directly assigned to SPI peripheral
        gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO5);
        gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO5);
        gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO4);

        gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);
        gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);
        gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);

        gpio_set_af(GPIOA, GPIO_AF5, GPIO5);  // SPI1_SCK
        gpio_set_af(GPIOB, GPIO_AF5, GPIO5);  // SPI1_MOSI
        gpio_set_af(GPIOB, GPIO_AF5, GPIO4);  // SPI1_MISO

        // CS Pin we drive manually
        gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO7);
        gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO7);
        gpio_set(GPIOD, GPIO7);

        // Setup SPI1
        rcc_periph_clock_enable(RCC_SPI1);
        spi_disable(SPI1);

        // The SPI peripheral is configured as a master with communication parameters
        // baudrate, data format 8/16 bits, frame format lsb/msb first, clock polarity
        // and phase. The SPI enable, CRC enable and CRC next controls are not affected. These must be controlled separately.
        spi_init_master(SPI1,
                        SPI_CR1_BAUDRATE_FPCLK_DIV_32,
                        SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                        SPI_CR1_CPHA_CLK_TRANSITION_1,
                        SPI_CR1_DFF_8BIT,
                        SPI_CR1_MSBFIRST);

        // Set hardware control of NSS pin. Because disabling it can cause master to become slave
        // depending on NSS input state.
        // Normally NSS will be hard-wired to slave. But in our case we have other pin connected to
        // slave CS and should drive it manually
        spi_enable_ss_output(SPI1);

        // In this example we will work with flash without using interrupts
	// So simply enable spi peripheral
	spi_enable(SPI1);
}


void cs_set(bool state)
{
	sk_pin_set(spiflash_ce, state);
}


void flash_tx(uint32_t len, const void *data)
{
        uint8_t *d = data;
        if ((!len) || (NULL == d))
                return;
        for (int32_t i = len - 1; i >= 0; i--) {
                spi_send(SPI1, d[i]);
                spi_read(SPI1);  // dummy read to provide delay
        }
}


void flash_rx(uint32_t len, const void *data)
{
        uint8_t *d = data;
        if (!len)
                return;
        for (int32_t i = len - 1; i >= 0; i--) {
                spi_send(SPI1, 0);  // dummy send to provide delay
                d[i] = spi_read(SPI1);
        }
}


struct __attribute__((packed, aligned(1)))
       __attribute__((scalar_storage_order("little-endian")))
       flash_jedec_id {
	// order matters here
                uint16_t device_id;
                uint8_t manufacturer;
};


int main(void)
{
        rcc_periph_clock_enable(RCC_GPIOE);
        rcc_periph_clock_enable(RCC_GPIOD);

        // LCD pins init
        sk_pin_group_mode_setup(lcd_group, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_rs, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_rw, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_en, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_bkl, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);

        sk_pin_mode_setup(led_green, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_set(led_green, false);

        clock_init();
        delay_timer_init();
        spi_init();

        cm_enable_interrupts();

        //LCD init
        sk_pin_group_set(lcd_group, 0x00);
        lcd_init_4bit(&lcd);
        sk_lcd_set_backlight(&lcd, true);
        sk_pin_set(led_green, true);

        while (1) {
                sk_pin_set(led_green, true);
                cs_set(0);
                const uint8_t cmd_jedec_id_get = 0x09F;
                flash_tx(1, &cmd_jedec_id_get);

                struct flash_jedec_id jedec_id = { 0 };
                flash_rx(sizeof(jedec_id), &jedec_id);

                cs_set(1);
                sk_pin_set(led_green, false);

                char buffer[40];
                sk_lcd_set_addr(&lcd, 0x00);
		snprintf(buffer, sizeof(buffer), "Manufacturer:%Xh\nSerial:%Xh",
                        (unsigned int)jedec_id.manufacturer, (unsigned int)jedec_id.device_id);
                lcd_clear_display(&lcd);
                lcd_print_text(&lcd, buffer);

		delay_ms(500);
        }
}
