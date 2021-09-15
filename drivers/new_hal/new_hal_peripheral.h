#ifndef NEW_HAL_PERIPHERAL_H
#define	NEW_HAL_PERIPHERAL_H

#ifdef _MIOSIX
#include "interfaces/arch_registers.h"
#include "interfaces/delays.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f4xx.h"
#endif //_MIOSIX

#include "usb_peripheral.h"
#include "drivers/new_hal/new_hal_memory.h"

#ifdef MIOSIX_DEFINE_BOARD_NAME

namespace mxusb {

/**
 * \internal
 * Hardware Abstraction Layer for the USB peripheral registers
 */
class USBperipheralImpl : public USBperipheral
{
public:
    void setAddress(unsigned short addr);

    void configureInterrupts();

    bool enable();

    void reset();

    void disable();

    void ep0setTxStatus(RegisterStatus status);

    void ep0setRxStatus(RegisterStatus status);

    unsigned short ep0read(unsigned char *data, int size = 0);

    void ep0reset();

    void ep0beginStatusTransaction();

    void ep0endStatusTransaction();

    bool ep0write(int size, const unsigned char *data = nullptr);
};

} //namespace mxusb

#endif //MIOSIX_DEFINE_BOARD_NAME

#endif //NEW_HAL_PERIPHERAL_H