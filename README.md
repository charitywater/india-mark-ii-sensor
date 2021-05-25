# charity: water - India Mark II Sensor

The software in this repository was developed and designed for the India Mark II handpump sensor for charity: water. This sensor will expand the number of wells compatible with charity: water’s IoT platform to include the India Mark II handpump. This sensor has the ability to monitor water flow on the IM2 handpump along with measure number of strokes and stroke length.  

## Directory Structure

- **am**: This folder contains the code for the application micro, which is a STM32.
- **ssm**: This folder contains the code for the sensor support micro, which is the MSP430 from TI.
- **shared**: This folder contains code that is shared between the SSM and the AM.  Specifically, the code that is used for the micros to talk to each other over the SPI interface.
- **amBootloader**: This folder contains the AM bootloader source code.

 

