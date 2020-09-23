/**
 * In this program is programed Color Sensor TCS3200.
 * Documentation about this sensor in documentation/color sensor/tcs3200-e11.pdf
 * This sensor has 5 input pins and 1 output.
 * s0, s1 - Output frequency scaling selection inputs.
 * s2, s3 - Photodiode type selection inputs.
 * OE - Enable for fo (active low).
 * OUT - Output frequency (fo).
 *
 * We set the pin group for s0, s1, s2, s3.
 * OUT transmit output frequency, so this frequency is read by freq_read() from freq_read.h
 * freq_read() was based on TIM3, TIM2 and TIM4.
 */
#include "color_sensor.h"
#include "pin.h"
#include "freq_read.h"

void color_get_rgb(sk_pin_group group, uint32_t ms, uint32_t *rgb_arr)
{
        //red
        sk_pin_group_set(group, 0b0011);
        rgb_arr[0] = freq_read(ms);
        //green
        sk_pin_group_set(group, 0b1111);
        rgb_arr[1] = freq_read(ms);
        //blue
        sk_pin_group_set(group, 0b1011);
        rgb_arr[2] = freq_read(ms);
}


uint8_t color_name(uint32_t *rgb_arr)
{
        if ((24 <= rgb_arr[0] && rgb_arr[0] <= 25)
                && (19 <= rgb_arr[1] && rgb_arr[1] <= 21)
                && (25 <= rgb_arr[2] && rgb_arr[2] <= 27)) {
                return 0;  // Red
        } else if ((23 <= rgb_arr[0] && rgb_arr[0] <= 25)
                && (22 <= rgb_arr[1] && rgb_arr[1] <= 24)
                && (26 <= rgb_arr[2] && rgb_arr[2] <= 28)) {
                return 1;  // Green
        } else if ((27 <= rgb_arr[0] && rgb_arr[0] <= 33)
                && (24 <= rgb_arr[1] && rgb_arr[1] <= 29)
                && (27 <= rgb_arr[2] && rgb_arr[2] <= 32)) {
                return 2;  // Yellow
        } else if ((26 <= rgb_arr[0] && rgb_arr[0] <= 32)
                && (20 <= rgb_arr[1] && rgb_arr[1] <= 23)
                && (25 <= rgb_arr[2] && rgb_arr[2] <= 28)) {
                return 3;  // Orange
        } else if ((21 <= rgb_arr[0] && rgb_arr[0] <= 23)
                && (19 <= rgb_arr[1] && rgb_arr[1] <= 21)
                && (26 <= rgb_arr[2] && rgb_arr[2] <= 28)) {
                return 4;  // Violet
        } else {
                return 5;
        }
}
