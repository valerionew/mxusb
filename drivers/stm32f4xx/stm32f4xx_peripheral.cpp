#include "stm32f4xx_peripheral.h"

#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

//
// class Endpoint
//

void EndpointRegister::IRQsetType(Type type)
{
    unsigned short reg=EPR;
    //Clear all toggle bits, so not to toggle any of them
    reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
           USB_EP0R_STAT_TX);
    //Avoid clearing an interrupt flag because of a read-modify-write
    reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    reg &= ~USB_EP0R_EP_TYPE;
    reg |= type;
    EPR=reg;
}

void EndpointRegister::IRQsetTxStatus(Status status)
{
    unsigned short reg=EPR;
    //Clear all toggle bits except STAT_TX, since we need to toggle STAT_TX
    reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX);
    //Avoid clearing an interrupt flag because of a read-modify-write
    reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    if(status & (1<<0)) reg ^=USB_EP0R_STAT_TX_0;
    if(status & (1<<1)) reg ^=USB_EP0R_STAT_TX_1;
    EPR=reg;
}

EndpointRegister::Status EndpointRegister::IRQgetTxStatus() const
{
    return static_cast<EndpointRegister::Status>((EPR>>4) & 0x3);
}

void EndpointRegister::IRQsetRxStatus(Status status)
{
    unsigned short reg=EPR;
    //Clear all toggle bits except STAT_RX, since we need to toggle STAT_RX
    reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_TX);
    //Avoid clearing an interrupt flag because of a read-modify-write
    reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    if(status & (1<<0)) reg ^=USB_EP0R_STAT_RX_0;
    if(status & (1<<1)) reg ^=USB_EP0R_STAT_RX_1;
    EPR=reg;
}

EndpointRegister::Status EndpointRegister::IRQgetRxStatus() const
{
    return static_cast<EndpointRegister::Status>((EPR>>12) & 0x3);
}

void EndpointRegister::IRQsetTxBuffer(shmem_ptr addr, unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+0)=addr & 0xfffe;
    SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
}

void EndpointRegister::IRQsetTxBuffer0(shmem_ptr addr, unsigned short size)
{
    IRQsetTxBuffer(addr,size);
}

void EndpointRegister::IRQsetTxBuffer1(shmem_ptr addr, unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+4)=addr & 0xfffe;
    SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
}

void EndpointRegister::IRQsetTxDataSize(unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
}

void EndpointRegister::IRQsetTxDataSize0(unsigned short size)
{
    IRQsetTxDataSize(size);
}

void EndpointRegister::IRQsetTxDataSize1(unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
}

void EndpointRegister::IRQsetRxBuffer(shmem_ptr addr, unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+4)=addr & 0xfffe;
    if(size>62)
    {
        size/=32;
        size--;
        size<<=10;
        size|=0x8000; //BL_SIZE=1
        SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
    } else {
        size<<=10;
        SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
    }
}

void EndpointRegister::IRQsetRxBuffer0(shmem_ptr addr, unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+0)=addr & 0xfffe;
    if(size>62)
    {
        size/=32;
        size--;
        size<<=10;
        size|=0x8000; //BL_SIZE=1
        SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
    } else {
        size<<=10;
        SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
    }
}

void EndpointRegister::IRQsetRxBuffer1(shmem_ptr addr, unsigned short size)
{
    IRQsetRxBuffer(addr,size);
}

unsigned short EndpointRegister::IRQgetReceivedBytes() const
{
    int ep=EPR & USB_EP0R_EA;
    return SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6) & 0x3ff;
}

unsigned short EndpointRegister::IRQgetReceivedBytes0() const
{
    int ep=EPR & USB_EP0R_EA;
    return SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2) & 0x3ff;
}

unsigned short EndpointRegister::IRQgetReceivedBytes1() const
{
    return IRQgetReceivedBytes();
}

void EndpointRegister::IRQclearTxInterruptFlag()
{
    unsigned short reg=EPR;
    //Clear all toggle bits, so not to toggle any of them.
    //Additionally, clear CTR_TX
    reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
            USB_EP0R_STAT_TX | USB_EP0R_CTR_TX);
    //Explicitly set CTR_RX to avoid clearing it due to the read-modify-write op
    reg |= USB_EP0R_CTR_RX;
    EPR=reg;
}

void EndpointRegister::IRQclearRxInterruptFlag()
{
    unsigned short reg=EPR;
    //Clear all toggle bits, so not to toggle any of them.
    //Additionally, clear CTR_RX
    reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
            USB_EP0R_STAT_TX | USB_EP0R_CTR_RX);
    //Explicitly set CTR_TX to avoid clearing it due to the read-modify-write op
    reg |= USB_EP0R_CTR_TX;
    EPR=reg;
}

void EndpointRegister::IRQsetEpKind()
{
    unsigned short reg=EPR;
    //Clear all toggle bits, so not to toggle any of them
    reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
           USB_EP0R_STAT_TX);
    //Avoid clearing an interrupt flag because of a read-modify-write
    reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    reg |= USB_EP0R_EP_KIND;
    EPR=reg;
}

void EndpointRegister::IRQclearEpKind()
{
    unsigned short reg=EPR;
    //Clear all toggle bits, so not to toggle any of them
    reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
           USB_EP0R_STAT_TX);
    //Avoid clearing an interrupt flag because of a read-modify-write
    reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    reg &= ~USB_EP0R_EP_KIND;
    EPR=reg;
}

void EndpointRegister::IRQsetDtogTx(bool value)
{
    unsigned short reg=EPR;
    //Clear all toggle bits except DTOG_TX, since we need to toggle it
    reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_STAT_RX | USB_EP0R_STAT_TX);
    //Avoid clearing an interrupt flag because of a read-modify-write
    reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    if(value) reg ^=USB_EP0R_DTOG_TX;
    EPR=reg;
}

void EndpointRegister::IRQtoggleDtogTx()
{
    unsigned short reg=EPR;
    //Clear all toggle bits except DTOG_TX, since we need to toggle it
    reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_STAT_RX | USB_EP0R_STAT_TX);
    //Avoid clearing an interrupt flag because of a read-modify-write
    reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX | USB_EP0R_DTOG_TX;
    EPR=reg;
}

bool EndpointRegister::IRQgetDtogTx() const
{
    return (EPR & USB_EP0R_DTOG_TX)!=0;
}

void EndpointRegister::IRQsetDtogRx(bool value)
{
    unsigned short reg=EPR;
    //Clear all toggle bits except DTOG_RX, since we need to toggle it
    reg &= ~(USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX | USB_EP0R_STAT_TX);
    //Avoid clearing an interrupt flag because of a read-modify-write
    reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    if(value) reg ^=USB_EP0R_DTOG_RX;
    EPR=reg;
}

void EndpointRegister::IRQtoggleDtogRx()
{
    unsigned short reg=EPR;
    //Clear all toggle bits except DTOG_RX, since we need to toggle it
    reg &= ~(USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX | USB_EP0R_STAT_TX);
    //Avoid clearing an interrupt flag because of a read-modify-write
    reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX | USB_EP0R_DTOG_RX;
    EPR=reg;
}

bool EndpointRegister::IRQgetDtogRx() const
{
    return (EPR & USB_EP0R_DTOG_RX)!=0;
}

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY