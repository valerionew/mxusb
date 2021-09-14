#ifndef STM32F4XX_PERIPHERAL_H
#define	STM32F4XX_PERIPHERAL_H

#ifdef _MIOSIX
#include "interfaces/arch_registers.h"
#include "interfaces/delays.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f4xx.h"
#endif //_MIOSIX

#include "usb_peripheral.h"
#include "drivers/stm32f4xx/stm32f4xx_memory.h"

#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

// STM does not have a constant that points to USB_OTG_DEVICE, so I had to create a constant myself
USB_OTG_DeviceTypeDef * const USB_OTG_DEVICE = (USB_OTG_DeviceTypeDef *) (USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE);

volatile uint32_t * const PCGCCTL = ((uint32_t*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE));

// Useful method to get an endpoint in struct
inline static USB_OTG_INEndpointTypeDef* EP_IN(unsigned char ep) {
    return (USB_OTG_INEndpointTypeDef*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + (ep << 5));
}

// Useful method to get an endpoint out struct
inline static USB_OTG_OUTEndpointTypeDef* EP_OUT(unsigned char ep) {
    return (USB_OTG_OUTEndpointTypeDef*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + (ep << 5));
}


/**
 * \internal
 * Hardware Abstraction Layer for the USB peripheral registers
 */
class USBperipheralImpl : public USBperipheral
{
public:
    void setAddress(unsigned short addr);

    void configureInterrupts();

    /**
     * \brief The enable method of the stm32f4 is supposed to support PERIPHERAL ONLY mode.
     * Hence, SRP and HNP are both disabled and the mode is forced to be peripheral only.
     */
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

private:
    void core_initialization();

    void power_on();

    void device_initialization();
};

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY

#endif //STM32F4XX_PERIPHERAL_H