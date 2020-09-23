servo - Initialization and installation of pulse width modulation for servo.
============================================================================

Initialization and installation of pulse width modulation for servo.
Servos connected to the PD12 and PD13.
OC channel for PD12 and PD13 TIM_OC1 and TIM_OC2.
For installation PWM we used TIM4.


.. c:autodoc:: inc/servo.h src/servo.c
   :clang: -I/lib/clang/10.0.0/include, -I../inc, -std=gnu18, -DHAWKMOTH
