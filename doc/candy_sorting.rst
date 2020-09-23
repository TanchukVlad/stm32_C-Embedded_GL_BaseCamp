candy_sorting - sorts candies in the correct cells depends on color of candy.
=============================================================================

This program sorts candies in the correct cells depends on color of candy.
For realisation this device, was used:
      Servo Motor SG90 - 2 pieces
      Color Sensor TCS3200 - 1 pieces
      GL Starter Kit with stm32f407VG - Discovery
 Color is read by Color Sensor which return some frequency for each
 component(R, G and B).
 Execution order:
      1) Servo 1 directs candy on the Color Sensor;
      2) Color Sensor recognize color;
      3) Servo 2 set slide on the correct cell depends on color;
      4) Servo 1 send candy on the slide.
 Program starting with calibration. You can set your cells in the correct place.
 If you was late, you can repeat calibration.
 During the program, you be able to make pause until you'll press button.

.. c:autodoc:: src/candy_sorting.c
   :clang: -I/lib/clang/10.0.0/include, -I../inc, -std=gnu18, -DHAWKMOTH
