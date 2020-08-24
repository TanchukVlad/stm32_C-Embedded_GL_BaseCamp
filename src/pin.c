#include "pin.h"


bool sk_pin_read(sk_pin pin)
{
	bool ret = gpio_port_read(sk_pin_port_to_gpio(pin.port)) & (1 << pin.pin);
	return ret ^ pin.isinverse;
}


void sk_pin_set(sk_pin pin, bool val)
{
	if (val ^ pin.isinverse)
		gpio_set(sk_pin_port_to_gpio(pin.port), (1 << pin.pin));
	else
		gpio_clear(sk_pin_port_to_gpio(pin.port), (1 << pin.pin));
}


void sk_pin_toggle(sk_pin pin)
{
	gpio_toggle(sk_pin_port_to_gpio(pin.port), (1 << pin.pin));
}


/**
 * Densification helper (private)
 * Example:
 *     mask = 0b1010000011110010
 *   sparse = 0b1010101010101010
 *   result = 0b1 1     1010  1
 */
uint16_t group_densify(uint16_t mask, uint16_t sparse)
{
	uint16_t res = 0;
	int idx = 0;
	for (int i = 0; i<16; i++)
		if (mask & (1 << i)) {
			res |= sparse & (1 << i) ? (1 << idx) : 0;
			idx++;
		}
	return res;
}


/**
 * Sparsification helper (private)
 * Example:
 *     mask = 0b1010000011110010
 *    dense = 0b1 1     1010  1
 *   result = 0b1010000010100010
 */
uint16_t group_sparsify(uint16_t mask, uint16_t dense)
{
	uint16_t res = 0;
	int idx = 0;
	for (int i = 0; i<16; i++)
		if (mask & (1 << i)) {
			res |= dense & (1 << idx) ? (1 << i) : 0;
			idx++;
		}
	return res;
}


uint16_t sk_pin_group_read(sk_pin_group group)
{
	uint16_t val = gpio_port_read(sk_pin_port_to_gpio(group.port));
	val ^= group.inversions;
	return group_densify(group.pins, val);  
}


void sk_pin_group_set(sk_pin_group group, uint16_t val)
{
	val = group_sparsify(group.pins, val);  
	val ^= group.inversions;
	//ODR: output data register
	volatile uint32_t *odr = &GPIO_ODR(sk_pin_port_to_gpio(group.port)); 
	uint32_t pval = *odr;
	pval &= ~((uint32_t)(group.pins));	// reset all pins in account
	pval |= val;			// set all pins in account to our values
	*odr = pval; 
}


void sk_pin_group_toggle(sk_pin_group group, uint16_t val)
{
	val = group_sparsify(group.pins, val);
	volatile uint32_t *odr = &GPIO_ODR(sk_pin_port_to_gpio(group.port));
	*odr ^= val;
}


void sk_pin_mode_setup(sk_pin pin, enum sk_mode mode)
{
	gpio_mode_setup(sk_pin_port_to_gpio(pin.port), mode, 0x0, (1 << pin.pin));
}





