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
 * color_read_freq() was based on TIMx.
 */
#include "color_sensor.h"


uint32_t color_read_freq(sk_pin pin, uint32_t ms)
{
        sk_tick_delay_ms(100);
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
                        // after cs_out 1 and 0 (it is one impulse) increment freq_cnt
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
                        scale = 10;//17;
                        break;
                case 'G':
                        scale = 10;//23;
                        break;
                case 'B':
                        scale = 10;//17;
                        break;
                default:
                        scale = 10;//10;
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
        //sk_pin_group_set(group, 0b0111);
        //sk_tick_delay_ms(10);
}


uint8_t color_name(uint8_t *rgb_arr)
{
        if ((28 <= rgb_arr[0] && rgb_arr[0] <= 30)
                && (22 <= rgb_arr[1] && rgb_arr[1] <= 23)
                && (29 <= rgb_arr[2] && rgb_arr[2] <= 31)) {
                return 0;  // Red
        } else if ((26 <= rgb_arr[0] && rgb_arr[0] <= 28)
                && (25 <= rgb_arr[1] && rgb_arr[1] <= 27)
                && (29 <= rgb_arr[2] && rgb_arr[2] <= 31)) {
                return 1;  // Green
        } else if ((30 <= rgb_arr[0] && rgb_arr[0] <= 35)
                && (27 <= rgb_arr[1] && rgb_arr[1] <= 31)
                && (31 <= rgb_arr[2] && rgb_arr[2] <= 34)) {
                return 2;  // Yellow
        } else if ((30 <= rgb_arr[0] && rgb_arr[0] <= 34)
                && (24 <= rgb_arr[1] && rgb_arr[1] <= 26)
                && (30 <= rgb_arr[2] && rgb_arr[2] <= 32)) {
                return 3;  // Orange
        } else if ((24 <= rgb_arr[0] && rgb_arr[0] <= 27)
                && (22 <= rgb_arr[1] && rgb_arr[1] <= 24)
                && (29 <= rgb_arr[2] && rgb_arr[2] <= 32)) {
                return 4;  // Violet
        } else {
                return 5;
        }
}
