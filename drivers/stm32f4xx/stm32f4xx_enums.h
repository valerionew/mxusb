#ifndef STM32F4XX_ENUMS_H
#define	STM32F4XX_ENUMS_H

#ifdef _MIOSIX
#include "interfaces/arch_registers.h"
#include "interfaces/delays.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f4xx.h"
#endif //_MIOSIX

#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

/**
 * Note: bitmask for Descriptor::Type (bitmask used in standard USB
 * descriptors) differ from Endpoint::Type (bitmask used in stm32's EPnR
 * register bits for endpoint types)
 */
enum RegisterType
{
    CONTROL=0,
    ISOCHRONOUS=1,
    BULK=2,
    INTERRUPT=3
};

enum RegisterStatus
{
    DISABLED=0,
    STALL=1,
    NAK=2,
    VALID=3
};

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY

#endif //STM32F1XX_ENUMS_H
