#ifndef STM32F4XX_CONFIG_H
#define STM32F4XX_CONFIG_H

#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

/// \internal
/// Number of hardware endpoints of the stm32
const int NUM_ENDPOINTS=3;

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY

#endif //STM32F4XX_CONFIG_H