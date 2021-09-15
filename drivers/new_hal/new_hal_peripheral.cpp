#include "new_hal_peripheral.h"

#ifdef _MIOSIX
#include "kernel/kernel.h"
using namespace miosix;
#else //_MIOSIX
#include "libraries/system.h"
#endif //_MIOSIX

#ifdef MIOSIX_DEFINE_BOARD_NAME

//
// interrupt handler
//

/**
 * \internal
 * Low priority interrupt, called for everything except double buffered
 * bulk/isochronous correct transfers.
 */
extern void OTG_FS_IRQHandler() __attribute__((naked));
void OTG_FS_IRQHandler()
{
    #ifdef _MIOSIX
    //Since a context switch can happen within this interrupt handler, it is
    //necessary to save and restore context
    saveContext();
    asm volatile("bl _ZN5mxusb13USBirqHandlerEv");
    restoreContext();
    #else //_MIOSIX
    asm volatile("ldr r0, =_ZN5mxusb13USBirqHandlerEv\n\t"
                 "bx  r0                             \n\t");
    #endif //_MIOSIX
}


namespace mxusb {

//
// class USBperipheral
//

void USBperipheralImpl::setAddress(unsigned short addr)
{
    // TODO: add here implementation of set address
}

void USBperipheralImpl::configureInterrupts()
{
    // TODO: add here configuration of interrupts

    //Configure interrupts
    NVIC_EnableIRQ(OTG_FS_IRQn);
    NVIC_SetPriority(OTG_FS_IRQn,3);//Higher priority (Max=0, min=15)
}

bool USBperipheralImpl::enable()
{
    // TODO: add here implementation of peripheral enable
    return false;
}

void USBperipheralImpl::reset()
{
    // TODO: add here implementation of reset of peripheral
}

void USBperipheralImpl::disable()
{
    // TODO: add here disabling of peripheral
}

void USBperipheralImpl::ep0setTxStatus(RegisterStatus status)
{
    // TODO: add here implementation of set tx status
}

void USBperipheralImpl::ep0setRxStatus(RegisterStatus status)
{
    // TODO: add here implementation of set rx status
}

unsigned short USBperipheralImpl::ep0read(unsigned char *data, int size)
{
    // TODO: add here read of received bytes count
    unsigned short readBytes = 0;

    // if buffer size is not specified, read all bytes
    if (size <= 0) {
        size = readBytes;
    }

    SharedMemory::instance().copyBytesFrom(data,0,size);
    return readBytes;
}

void USBperipheralImpl::ep0reset()
{
    // TODO: add here implementation of reset of endpoint 0
}

void USBperipheralImpl::ep0beginStatusTransaction()
{
    // NOTE: empty method
    // implemented in F1 driver
}

void USBperipheralImpl::ep0endStatusTransaction()
{
    // NOTE: empty method
    // implemented in F1 driver
}

bool USBperipheralImpl::ep0write(int size, const unsigned char *data)
{
    // TODO: add here write of packet to be sent

    // push packet to TX FIFO if it is not a zero-length packet
    if (size > 0) {
        SharedMemory::instance().copyBytesTo(0,data,size);
    }
    
    return true;
}

} //namespace mxusb

#endif //MIOSIX_DEFINE_BOARD_NAME