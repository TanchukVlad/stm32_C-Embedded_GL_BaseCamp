# stm32_C-Embedded_GL_BaseCamp
STM32 C/Embedded Global Logic BaseCamp. 
This repository is for my training and developments with stm32f4DISCOVERY board and with GL Extension board.

The repository consists of final project: **Candy_Sorting**


## About Candy_Sorting project 
This project alows to sort candies into the correct cells depending on the color of candy.
Modules used for realisation of the device include:

     1. Servo Motor SG90 - 2 pieces
     2. Color Sensor TCS3200 - 1 pieces
     3. GL Starter Kit with stm32f407VG - Discovery
Color is read by Color Sensor which returns certain frequency for each
component(R, G and B).
Execution order:

     1. Servo 1 directs the candy on the Color Sensor;
     2. The Color Sensor recognizes color;
     3. Servo 2 sets slide on the correct cell depending on the color;
     4. Servo 1 sends the candy to the slide.
Program starts with calibration. You can set your cells in the correct place.
If you were late, you can repeat calibration.
During the program, you are able to make pause until you press button.


## How to start
Recursively clone the repository:
```
git clone --recursive https://github.com/TanchukVlad/stm32_C-Embedded_GL_BaseCamp
```
You can flash the project on your STM32F4DISCOVERY using command:
```
make target flash
```


## Auto-generate documentation sphinx hawkmoth
You can create project documentation. Go to folder: **/doc** and use command:
```
make html
```


## Useful links

     1. [libopencm3 Developer Documentation](http://libopencm3.org/docs/latest/html/)
     2. [STM32F407VG STMicroelectronics Resources page](https://www.st.com/en/microcontrollers-microprocessors/stm32f407vg.html#resource)
     3. [STM32F4DISCOVERY STMicroelectronics Resources page](https://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-mpu-eval-tools/stm32-mcu-mpu-eval-tools/stm32-discovery-kits/stm32f4discovery.html#resource)


## License
Everything in this repository, except the STMicroelectronics documentation is licensed under the MIT License.
See LICENSE for details.

For more on STMicroelectronics documentation licensing consider their official website (https://st.com)




     
