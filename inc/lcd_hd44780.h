/**
 * libsk GL-SK on-board LCD abstraction layer
 *
 * The display is WH1602B (based on HD44780 controller)
 */
#include "pin.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


/** Pointer to delay(uint32_t var) function defined as type */
typedef void (*sk_delay_func_t)(uint32_t);

struct sk_lcd {
        /** HD44780 data pins (DB0..DB7 or DB4..DB7). */
        sk_pin_group *pin_group_data;
        /**
         * HD44780 Selects registers.
         * 0: Instruction register (for write) Busy flag: address counter (for read).
         * 1: Data register (for write and read).
         */
        sk_pin *pin_rs;
        /** HD44780 Selects read or write. 0 -> Write, 1 -> Read. */
        sk_pin *pin_rw;
        /** Starts data read/write.*/
        sk_pin *pin_en;
        /** Display backlight pin. Set to NULL if not used */
        sk_pin *pin_bkl;
        /**
         * Pointer to backlight control function (i.e. to set backlight LED PWM level).
         * Set to NULL if not used. This way only two levels will be possible
         * (0 for OFF and != 0 for ON)
         */
        //void (*set_backlight_func)(uint8_t);
        /**
         * Pointer to user-provided delay function with microsecond resolution.
	 * Set to NULL to use ms delay as a fallback
         */
        sk_delay_func_t delay_func_us;
        /**
         * Pointer to user-provided delay function with millisecond resolution.
	 * Set to NULL to use ms delay as a fallback
         */
        sk_delay_func_t delay_func_ms;
        /** True for 4-bit HD44780 interface, False for 8-bit. Only 4-bit IF is supported for now */
        unsigned int is4bitinterface : 1;
};

/**
 * Set backlight on the lcd.
 * @mode: true - on, fasle - off backlight.
 */
void sk_lcd_set_backlight(struct sk_lcd *lcd, bool mode);

/**
 * Display ON/OFF Control.
 * @d: Set display on/off control bit.
 * @c: Set cursor on/off control bit..
 * @b: Set blinking on/off control bit..
 *
 * Context: Set display (D), cursor (C), and blinking of cursor (B) on/off control bit.
 */
void lcd_display_on_off_control(struct sk_lcd *lcd, bool d, bool c, bool b);

/**
 * Clear Display.
 *
 * Context: Write “00H” to DDRAM and set DDRAM address to “00H” from AC.
 */
void lcd_clear_display(struct sk_lcd *lcd);

/**
 * Initializing of LCD.
 *
 * Context: Initializing of LCD for 4-Bit Ineterface.
 */
void lcd_init_4bit(struct sk_lcd *lcd);

/**
 * Write Data to RAM.
 * @byte: Will be written into the RAM.
 *
 * Context: Write data into internal RAM (DDRAM/CGRAM).
 */
void lcd_write_data(struct sk_lcd *lcd, uint8_t byte);

/**
 * Entry Mode Set.
 * @id: decrement/increment cnt (I/D).
 * @sh: display noshift / shift (SH).
 *
 * Context: Assign cursor moving direction and enable the shift of entire display.
 */
void lcd_entry_mode_set(struct sk_lcd *lcd, bool id, bool sh);

/**
 * Print text on lcd
 * @text: String of text which will be written.
 *
 * Context: Function writes string on the lcd display. Can switch row.
 */
void lcd_print_text(struct sk_lcd *lcd, char *text);


/**
 * Set DDRAM Address
 * @byte: Address which will be set.
 *
 * Context: Set DDRAM address in address counter.
 */
void sk_lcd_set_addr(struct sk_lcd *lcd, uint8_t byte);
