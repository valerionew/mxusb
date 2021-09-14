#ifndef USB_PERIPHERAL_H
#define	USB_PERIPHERAL_H

#include "usb_hal_include.h"

namespace mxusb {

class USBperipheralImpl;

class USBperipheral
{
public:
    static USBperipheral& instance();

    USBperipheral() {}

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

private:
    USBperipheral(const USBperipheral&);
    USBperipheral& operator= (const USBperipheral&);

    USBperipheral(USBperipheralImpl *impl);

    USBperipheralImpl *pImpl;
};
    
} //namespace mxusb


#endif //USB_PERIPHERAL_H