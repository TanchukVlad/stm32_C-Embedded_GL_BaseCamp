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

//sk_err sk_lcd_set_backlight(struct sk_lcd *lcd, uint8_t level);
void lcd_poweron_delay(struct sk_lcd *lcd);


void sk_lcd_set_backlight(struct sk_lcd *lcd, bool mode);


void lcd_display_on_off_control(struct sk_lcd *lcd, bool d, bool c, bool b);


void lcd_clear_display(struct sk_lcd *lcd);


void lcd_init_4bit(struct sk_lcd *lcd);


void lcd_write_data(struct sk_lcd *lcd, uint8_t byte);


void lcd_entry_mode_set(struct sk_lcd *lcd, bool id, bool sh);


void lcd_print_text(struct sk_lcd *lcd, char *text);


void sk_lcd_set_addr(struct sk_lcd *lcd, uint8_t byte);
