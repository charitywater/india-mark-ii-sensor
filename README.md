# charity: water - India Mark II

The software in this repository was developed and designed for the India Mark II handpump sensor for Charity Water. This sensor will expand the number of wells compatible with Charity Water’s IOT platform to include the India Mark II handpump. This sensor has the ability to monitor water flow on the IM2 handpump along with number of strokes and stroke length.  Ultimately, the goal is to start to get to a "predictive maintenance" solution for communitities to prevent wells breaking down and being unusable.

## Directory Structure

- **am**: This folder contains the code for the application micro, which is the STM32.
- **ssm**: This folder contains the code for the sensor support micro, which is the MSP430 from TI.
- **shared**: This folder contains code that is shared between the SSM and the AM.  Specifically, the code that is used for the micros to talk to each other over the SPI interface.
- **amBootloader**: This folder contains the AM bootloader source code.

 

