#ifndef STM32F1XX_CONFIG_H
#define STM32F1XX_CONFIG_H

#ifdef _BOARD_STM32F103C8_BREAKOUT

namespace mxusb {

/// \internal
/// Number of hardware endpoints of the stm32
const int NUM_ENDPOINTS=8;

} //namespace mxusb

#endif //_BOARD_STM32F103C8_BREAKOUT

#endif //STM32F1XX_CONFIG_H