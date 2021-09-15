#ifndef NEW_HAL_CONFIG_H
#define NEW_HAL_CONFIG_H

#ifdef MIOSIX_DEFINE_BOARD_NAME

namespace mxusb {

/// \internal
/// Number of hardware endpoints of the stm32
const int NUM_ENDPOINTS=3;

} //namespace mxusb

#endif //MIOSIX_DEFINE_BOARD_NAME

#endif //NEW_HAL_CONFIG_H