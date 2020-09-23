color_sensor - In this program is programed Color Sensor TCS3200.
=================================================================

Documentation about this sensor in documentation/color sensor/tcs3200-e11.pdf
This sensor has 5 input pins and 1 output.
        s0, s1 - Output frequency scaling selection inputs.
        s2, s3 - Photodiode type selection inputs.
        OE - Enable for fo (active low).
        OUT - Output frequency (fo).

We set the pin group for s0, s1, s2, s3.
OUT transmit output frequency, so this frequency is read by freq_read() from freq_read.h
freq_read() was based on TIM3, TIM2 and TIM4.

.. c:autodoc:: inc/color_sensor.h src/color_sensor.c
   :clang: -I/lib/clang/10.0.0/include, -I../inc, -std=gnu18, -DHAWKMOTH
