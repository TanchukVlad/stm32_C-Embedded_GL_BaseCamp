/**
 * Pin abstraction over libopencm3/stm32
 * Using libopencm3/stm32/gpio.h
 */

#include "macro.h"
#include <libopencm3/stm32/gpio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * GPIO ports definition from port A to port H
 */
enum sk_port {
	PORTA = 0,
	PORTB = 1,
	PORTC = 2,
	PORTD = 3,
	PORTE = 4,
	PORTF = 5,
	PORTG = 6,
	PORTH = 7
};

enum sk_mode {
	MODE_INPUT = GPIO_MODE_INPUT,
	MODE_OUTPUT = GPIO_MODE_OUTPUT,
	MODE_AF = GPIO_MODE_AF,
	MODE_ANALOG = GPIO_MODE_ANALOG
};


/**
 * GPIO pin definition
 */
struct sk_attr_pack(1) sk_pin {
	/** Port value */
	uint8_t port : 3;  // 8 ports, so 3-bit enough for packing -> 2^3 = 8
	/** Pin number (0 ... 15) */
	uint8_t pin : 4;  // 16 pins, so 4-bit enogh for packing -> 2^4 = 16
	/** If port should be inversted, will be set to true */
	uint8_t isinverse : 1;
};


/**
 * GPIO pin_group definition
 * Represents group of pins residing in the same GPIO port.
 */
struct sk_attr_pack(1) sk_pin_group {
	/** Port value */
	uint16_t port : 3;  // 8 ports, so 3-bit enough for packing -> 2^3 = 8
	/** For future use */
	uint16_t __reserved : 13;
	/** 16-bit value where each bit represents corresponding pin in a port */
	uint16_t pins;
	/** 16-bit mask where each bit==1 represents inversion in :c:member:`sk_pin_group.pins` */
	uint16_t inversions;
};


typedef struct sk_pin sk_pin;
typedef struct sk_pin_group sk_pin_group;


/**
 * Take port number and return base for port.
 * Map sk_port definitions to libopencm3 GPIOx. Intended mainly for private use.
 * Faster and more concise than building 1:1 map.
 * Benefit from memory locations of GPIO registers.
 * GPIO_PORT_B_BASE - GPIO_PORT_A_BASE size between PORTA and PORTB multiply by port number
 * and sum on PORTA
 * Result: we have base for our port
 */
inline sk_attr_alwaysinline uint32_t sk_pin_port_to_gpio(enum sk_port port)
{
	return GPIO_PORT_A_BASE + (GPIO_PORT_B_BASE - GPIO_PORT_A_BASE) * port;
}


/**
 * Read pin input and return its value
 * @pin: Pin wich have to read
 * @return: Boolean value of pin input
 *
 * Note:
 * For the means of improved speed, directly reads port input register and performs no checks
 * that the pin is really set to input.
 * The user is responsible for checking pin configuration validity.
 *
 * Inversion is taken into account as specified in :c:type:`sk_pin`
 */
bool sk_pin_read(sk_pin pin);


/**
 * Set pin output level to specified value
 *
 * Note:
 * Does not check if pin is set to output, to improve speed.
 * The user is responsible for providing a correct pin as an argument.
 *
 * Inversion is taken into account as specified in :c:type:`sk_pin`
 */
void sk_pin_set(sk_pin pin, bool val);


/**
 * Toggle pin output level to the opposite value
 *
 * Note:
 * Does not check if pin is set to output, for speed improvement.
 * The user is responsible for providing a correct pin.
 *
 * Inversion is taken into account as specified in :c:type:`sk_pin`
 */
void sk_pin_toggle(sk_pin pin);


/**
 * Read group of pins and return collected value.
 * @group: pin group (:c:type:`sk_pin_group`)
 * @return: densified value read from specified :c:member:`sk_pin_group.pins`
 *
 * Only the pins specified in :c:type:`sk_pin_group` are collected in the resulting value applying
 * densification.
 * If :c:member:`sk_pin_group.pins` are specified, for example as 0b0010100100001100,
 * and GPIO has value 0bABCDEFGHIJKLMNOP, the resulting (collected) value will be
 * 0bCEHMN.
 */
uint16_t sk_pin_group_read(sk_pin_group group);


/**
 * Set group of pins to the provided (densified) value
 * @group: pin group (:c:type:`sk_pin_group`)
 * @values: densified value specifying which pins to set
 *
 * Only the pins specified in :c:type:`sk_pin_group` are affected by set operation.
 * If :c:member:`sk_pin_group.pins` are specified, for example as 0b0101000000000011,
 * and `values` is set to 0b000011110000ABCD, the resulting GPIO value will be
 * 0b0A0B0000000000CD.
 *
 * Note:
 * Access to the corresponding GPIO registers is not atomic.
 */
void sk_pin_group_set(sk_pin_group group, uint16_t val);


/**
 * Toggle group of pins to the opposite values
 * @group: pin group (:c:type:`sk_pin_group`)
 * @values: densified value specifying which pins to toggle
 *
 * Only the pins specified in :c:type:`sk_pin_group` are affected by set operation.
 * If :c:member:`sk_pin_group.pins` are specified, for example as 0b0101000000000011,
 * and `values` is set to 0b000011110000ABCD, the resulting GPIO value will be
 * 0b0A0B0000000000CD.
 *
 * Note:
 * Access to the corresponding GPIO registers is not atomic.
 */
void sk_pin_group_toggle(sk_pin_group group, uint16_t val);


/**
 * Set pin mode
 * @mode: pin mode is taken from libopencm3
 *
 * Doesn't set pull up and pull down.
 */
void sk_pin_mode_setup(sk_pin pin, enum sk_mode mode);


/**
 * Set pin group mode
 * @mode: pin mode is taken from libopencm3
 *
 * Doesn't set pull up and pull down.
 */
void sk_pin_group_mode_setup(sk_pin_group group, enum sk_mode mode);
