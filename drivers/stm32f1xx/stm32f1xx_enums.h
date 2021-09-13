#ifndef STM32F1XX_ENUMS_H
#define	STM32F1XX_ENUMS_H

#ifdef _MIOSIX
#include "interfaces/arch_registers.h"
#include "interfaces/delays.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f10x.h"
#endif //_MIOSIX

#ifdef _BOARD_STM32F103C8_BREAKOUT

namespace mxusb {

/**
 * Note: bitmask for Descriptor::Type (bitmask used in standard USB
 * descriptors) differ from Endpoint::Type (bitmask used in stm32's EPnR
 * register bits for endpoint types)
 */
enum RegisterType
{
    BULK=0,
    CONTROL=USB_EP0R_EP_TYPE_0,
    ISOCHRONOUS=USB_EP0R_EP_TYPE_1,
    INTERRUPT=USB_EP0R_EP_TYPE_1 | USB_EP0R_EP_TYPE_0
};

enum RegisterStatus
{
    DISABLED=0,
    STALL=1<<0,
    NAK=1<<1,
    VALID=(1<<0) | (1<<1)
};

} //namespace mxusb

#endif //_BOARD_STM32F103C8_BREAKOUT

#endif //STM32F1XX_ENUMS_H
