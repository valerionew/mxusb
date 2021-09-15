## How to create a driver for a new board
To add the support to a new board that is fully integrated with the Hardware Abstraction Layer (HAL) defined by the library, simply execute the following steps:

* create a copy of the `drivers/new_hal` folder
* rename all occurences of `new_hal` with the name of the new supported board
* in the headers file, update the `#ifdef MIOSIX_DEFINE_BOARD_NAME` preprocessor statements with the constant name specified by the Miosix kernel
* implement the required code, specific for the new hardware version, where marked with the predefined comments (`// TODO: add here ...`)
* register the source files for the new driver in the `Makefile` where indicated