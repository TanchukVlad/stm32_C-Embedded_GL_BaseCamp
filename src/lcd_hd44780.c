#include "lcd_hd44780.h"
#include <stddef.h>

static const uint32_t DELAY_ENA_STROBE_US = 1;
// Delay for the first and the second step of initialize
static const uint32_t DELAY_INIT0_US = 4100;
static const uint32_t DELAY_INIT1_US = 100;
// Entry Mode Set, Display ON/OFF Control, Cursor or Display Shift,
// Function Set, Set CGRAM Address, Set DDRAM Address commands
static const uint32_t DELAY_CONTROL_US = 39;
// Clear Display and Return Home commands
static const uint32_t DELAY_CLRRET_US = 1530;
// Read Data from RAM and Write Data to RAM commands
static const uint32_t DELAY_READWRITE_US = 43;
// Read Busy Flag and Address command
static const uint32_t DELAY_BUSYFLAG_US = 0;


/**
  * Private: Provides abstaction over two delay functions passed when constructing sk_lcd object
  *
  * Detect if optimal delay function is applicable and use it when possible with fallback
  * to unoptimal variants
  */
static void lcd_delay_us(struct sk_lcd *lcd, uint32_t us)
{
	if (NULL == lcd)
		return;

	sk_delay_func_t msfunc = lcd->delay_func_ms,
					usfunc = lcd->delay_func_us;

	if ((NULL == msfunc) && (NULL == usfunc))
		return;

	if (NULL == msfunc) {
		// only us-resolution func is set -> use unoptimal us delay
		usfunc(us);
		return;
	}

	if (NULL == usfunc) {
		// only ms-resolution func is set -> use rounded us delay
		msfunc((us % 1000) ? ((1 + us) / 1000) : (us / 1000));
		return;
	}

	// both functions are set -> use ms delay for divisor and us for remainder
	if (us / 1000)
		msfunc(us / 1000);
	if (us % 1000)
		usfunc(us % 1000);
}


static void lcd_data_set_halfbyte(struct sk_lcd *lcd, uint8_t half)
{
        sk_pin_set(*lcd -> pin_en, true);
        sk_pin_group_set(*lcd -> pin_group_data, half & 0x0F);
        lcd_delay_us(lcd, DELAY_ENA_STROBE_US);
        sk_pin_set(*lcd -> pin_en, true);
        lcd_delay_us(lcd, DELAY_ENA_STROBE_US);
}


static void lcd_data_set_byte(struct sk_lcd *lcd, uint8_t byte)
{
        if (lcd -> is4bitinterface) {
                lcd_data_set_halfbyte(lcd, byte >> 4);
                lcd_data_set_halfbyte(lcd, byte & 0x0F);
        } else {
                sk_pin_set(*lcd -> pin_en, true);
                sk_pin_group_set(*lcd -> pin_group_data, byte);
                lcd_delay_us(lcd, DELAY_ENA_STROBE_US);
        }
}


static void lcd_rsrw_set(struct sk_lcd *lcd, bool rs, bool rw)
{
	sk_pin_set(*lcd->pin_rs, rs);
	sk_pin_set(*lcd->pin_rw, rw);
}


static void lcd_send_byte(struct sk_lcd *lcd, bool rs, uint8_t byte)
{
	lcd_rsrw_set(lcd, rs, true);
	lcd_data_set_byte(lcd, byte);
}


/**
 * Set display on/off:
 * bit2 -- display on (D)
 * bit1 -- cursor on (C)
 * bit 0 -- blink on (B)
 */
void display_on_off_control(struct sk_lcd *lcd)
{
	lcd_rsrw_set(lcd, 0, 0);
	lcd_data_set_byte(lcd, 0b00001000 | 0b111);
	lcd_delay_us(lcd, DELAY_CONTROL_US);
}


void lcd_clear_display(struct sk_lcd *lcd)
{
	lcd_rsrw_set(lcd, 0, 0);
	lcd_data_set_byte(lcd, 0b00000001);
	lcd_delay_us(lcd, DELAY_CLRRET_US);
}


/**
 * Initial function for HD44780.
 */
void lcd_init_4bit(struct sk_lcd *lcd)
{
	lcd_rsrw_set(lcd, 0, 0);
	lcd_data_set_halfbyte(lcd, 0b0011);
	lcd_delay_us(lcd, DELAY_INIT0_US);

	lcd_rsrw_set(lcd, 0, 0);
	lcd_data_set_halfbyte(lcd, 0b0010);
	lcd_delay_us(lcd, DELAY_INIT1_US);

	lcd_rsrw_set(lcd, 0, 0);
	lcd_data_set_halfbyte(lcd, 0b0010);
	lcd_delay_us(lcd, DELAY_CONTROL_US);

        // Set display on/off
        display_on_off_control(lcd);

	// clear display
	lcd_clear_display(lcd);

	// entry mode set:
        // bit1 -- decrement/increment cnt (I/D),
        // bit0 -- display noshift / shift (SH)
	lcd_rsrw_set(lcd, 0, 0);
	lcd_data_set_byte(lcd, 0b00000100 | 0b10);
	lcd_delay_us(lcd, DELAY_CLRRET_US);
}


void lcd_write_data(struct sk_lcd *lcd, uint8_t byte)
{
	lcd_rsrw_set(lcd, 1, 0);
	lcd_data_set_byte(lcd, byte);
	lcd_delay_us(lcd, DELAY_CONTROL_US);
}
