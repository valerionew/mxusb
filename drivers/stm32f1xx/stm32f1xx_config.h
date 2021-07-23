#ifndef STM32F1XX_CONFIG_H
#define STM32F1XX_CONFIG_H

#ifdef _BOARD_STM32F103C8_BREAKOUT

namespace mxusb {

/// \internal
/// Number of hardware endpoints of the stm32
const int NUM_ENDPOINTS=8;

/// Size of buffer for endpoint zero. This constant is put here because it
/// must be accessible also from user code, to fill in the value in the device
/// descriptor. The USB standard specifies that only 8,16,32,64 are valid values
const unsigned short EP0_SIZE=32;

} //namespace mxusb

#endif //_BOARD_STM32F103C8_BREAKOUT

#endif //STM32F1XX_CONFIG_H