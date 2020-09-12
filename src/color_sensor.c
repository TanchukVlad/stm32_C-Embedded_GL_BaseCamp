/**
 * In this program is programed Color Sensor TCS3200.
 * Documentation about this sensor in documentation/color sensor/tcs3200-e11.pdf
 * This sensor has 5 input pins and 1 output.
 * s0, s1 - Output frequency scaling selection inputs.
 * s2, s3 - Photodiode type selection inputs.
 * OE - Enable for fo (active low).
 * OUT - Output frequency (fo).
 *
 * We set the pin group for s0, s1, s2, s3, OE.
 * OUT transmit output frequency, so this frequency is read by read_freq().
 * read_freq() was based on TIMx.
 */
#include "color_sensor.h"


uint32_t color_read_freq(sk_pin pin, uint32_t ms)
{
        uint32_t cur = sk_tick_get_current();
        uint32_t delta = (sk_tick_get_rate_hz() / 1000) * ms;
        uint32_t next = cur + delta;

        uint32_t freq_cnt = 0;

        if (sk_tick_get_current() > next) {
                while (sk_tick_get_current() > next) {
                        // wait cs_out = 1
                        while (!sk_pin_read(pin));
                        // wait cs_out = 0
                        while (sk_pin_read(pin));
                        //after cs_out 1 and 0 (it is one impulse) increment freq_cnt
                        freq_cnt++;
                }
        } else {
                while (sk_tick_get_current() <= next) {
                        while (!sk_pin_read(pin));

                        while (sk_pin_read(pin));

                        freq_cnt++;
                }
        }

        return freq_cnt / ms;
}


uint8_t color_scale(uint32_t freq, char color)
{
        uint8_t scale;

        switch (color) {
                case 'R':
                        scale = 17;
                        break;
                case 'G':
                        scale = 23;
                        break;
                case 'B':
                        scale = 17;
                        break;
                default:
                        scale = 10;
                        break;
        }

        return ((freq * scale) / 10 > 255) ? 255 : (freq * scale) / 10;
}


void color_get_rgb(sk_pin_group group, sk_pin pin, uint32_t ms, uint8_t *rgb_arr)
{
        //red
        sk_pin_group_set(group, 0b0011);
        rgb_arr[0] = color_scale(color_read_freq(pin, ms), 'R');
        //green
        sk_pin_group_set(group, 0b1111);
        rgb_arr[1] = color_scale(color_read_freq(pin, ms), 'G');
        //blue
        sk_pin_group_set(group, 0b1011);
        rgb_arr[2] = color_scale(color_read_freq(pin, ms), 'B');
        //clear
        sk_pin_group_set(group, 0b0111);
        sk_tick_delay_ms(10);
}


char color_name(uint8_t *rgb_arr)
{
        if ((149 < rgb_arr[0] && rgb_arr[0] < 256) && (59 < rgb_arr[1] && rgb_arr[1] < 76) && (44 < rgb_arr[2] && rgb_arr[2] < 76)) {
                return 'R';  // Red
        } else if ((69 < rgb_arr[0] && rgb_arr[0] < 101) && (80 < rgb_arr[1] && rgb_arr[1] < 256) && (60 < rgb_arr[2] && rgb_arr[2] < 95)) {
                return 'G';  // Green
        } else if ((40 < rgb_arr[0] && rgb_arr[0] < 80) && (70 < rgb_arr[1] && rgb_arr[1] < 170) && (100 < rgb_arr[2] && rgb_arr[2] < 256)) {
                return 'B';  // Blue
        } else if ((180 < rgb_arr[0] && rgb_arr[0] < 220) && (90 < rgb_arr[1] && rgb_arr[1] < 105) && (75 < rgb_arr[2] && rgb_arr[2] < 90)) {
                return 'O';  // Orange
        } else if ((200 < rgb_arr[0] && rgb_arr[0] < 256) && (175 < rgb_arr[1] && rgb_arr[1] < 256) && (65 < rgb_arr[2] && rgb_arr[2] < 125)) {
                return 'Y';  // Yellow
        } else if ((0 < rgb_arr[0] && rgb_arr[0] < 50) && (0 < rgb_arr[1] && rgb_arr[1] < 50) && (0 < rgb_arr[2] && rgb_arr[2] < 50)) {
                return 'b';  // black
        }

        return '\0';
}
