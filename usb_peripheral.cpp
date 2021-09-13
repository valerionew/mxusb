#include "usb_peripheral.h"

#include "drivers/stm32f1xx/stm32f1xx_peripheral.h"
#include "drivers/stm32f4xx/stm32f4xx_peripheral.h"

namespace mxusb {

USBperipheral& USBperipheral::instance()
{
    static USBperipheralImpl implementation;
    static USBperipheral singleton(&implementation);
    return singleton;
}

void USBperipheral::setAddress(unsigned short addr)
{
    pImpl->setAddress(addr);
}

void USBperipheral::configureInterrupts()
{
    pImpl->configureInterrupts();
}

bool USBperipheral::enable()
{
    return pImpl->enable();
}

void USBperipheral::reset()
{
    pImpl->reset();
}

void USBperipheral::disable()
{
    pImpl->disable();
}

void USBperipheral::ep0setTxStatus(RegisterStatus status)
{
    pImpl->ep0setTxStatus(status);
}

void USBperipheral::ep0setRxStatus(RegisterStatus status)
{
    pImpl->ep0setRxStatus(status);
}

unsigned short USBperipheral::ep0read(unsigned char *data, int size)
{
    return pImpl->ep0read(data, size);
}

void USBperipheral::ep0reset()
{
    pImpl->ep0reset();
}

void USBperipheral::ep0beginStatusTransaction()
{
    pImpl->ep0beginStatusTransaction();
}

void USBperipheral::ep0endStatusTransaction()
{
    pImpl->ep0endStatusTransaction();
}

bool USBperipheral::ep0write(int size, const unsigned char *data)
{
    return pImpl->ep0write(size, data);
}

USBperipheral::USBperipheral(USBperipheralImpl *impl) : pImpl(impl) {}
    
} //namespace mxusb
