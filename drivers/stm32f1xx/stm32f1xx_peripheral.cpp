#include "stm32f1xx_peripheral.h"

#ifdef _MIOSIX
#include "kernel/kernel.h"
using namespace miosix;
#else //_MIOSIX
#include "libraries/system.h"
#endif //_MIOSIX

#ifdef _BOARD_STM32F103C8_BREAKOUT

//
// interrupt handler
//

/**
 * \internal
 * Low priority interrupt, called for everything except double buffered
 * bulk/isochronous correct transfers.
 */
//extern void USB_LP_CAN1_RX0_IRQHandler() __attribute__((naked));
void __attribute__((naked)) USB_LP_CAN1_RX0_IRQHandler()
{
    #ifdef _MIOSIX
    //Since a context switch can happen within this interrupt handler, it is
    //necessary to save and restore context
    saveContext();
    asm volatile("bl _ZN5mxusb15USBirqLpHandlerEv");
    restoreContext();
    #else //_MIOSIX
    asm volatile("ldr r0, =_ZN5mxusb15USBirqLpHandlerEv\n\t"
                 "bx  r0                               \n\t");
    #endif //_MIOSIX
}

/**
 * \internal
 * High priority interrupt, called for a correct transfer on double bufferes
 * bulk/isochronous endpoint.
 */
//extern void USB_HP_CAN1_TX_IRQHandler() __attribute__((naked));
void __attribute__((naked)) USB_HP_CAN1_TX_IRQHandler()
{
    #ifdef _MIOSIX
    //Since a context switch can happen within this interrupt handler, it is
    //necessary to save and restore context
    saveContext();
    asm volatile("bl _ZN5mxusb15USBirqHpHandlerEv");
    restoreContext();
    #else //_MIOSIX
    asm volatile("ldr r0, =_ZN5mxusb15USBirqHpHandlerEv\n\t"
                 "bx  r0                               \n\t");
    #endif //_MIOSIX
}


namespace mxusb {

//
// class EndpointRegister
//

void EndpointRegister::IRQsetType(RegisterType type)
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

void EndpointRegister::IRQsetTxStatus(RegisterStatus status)
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

RegisterStatus EndpointRegister::IRQgetTxStatus() const
{
    return static_cast<RegisterStatus>((EPR>>4) & 0x3);
}

void EndpointRegister::IRQsetRxStatus(RegisterStatus status)
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

RegisterStatus EndpointRegister::IRQgetRxStatus() const
{
    return static_cast<RegisterStatus>((EPR>>12) & 0x3);
}

void EndpointRegister::IRQsetTxBuffer(shmem_ptr addr, unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+0)=addr & 0xfffe;
    SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
}

void EndpointRegister::IRQsetTxBuffer0(shmem_ptr addr, unsigned short size)
{
    IRQsetTxBuffer(addr,size);
}

void EndpointRegister::IRQsetTxBuffer1(shmem_ptr addr, unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+4)=addr & 0xfffe;
    SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
}

void EndpointRegister::IRQsetTxDataSize(unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
}

void EndpointRegister::IRQsetTxDataSize0(unsigned short size)
{
    IRQsetTxDataSize(size);
}

void EndpointRegister::IRQsetTxDataSize1(unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
}

void EndpointRegister::IRQsetRxBuffer(shmem_ptr addr, unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+4)=addr & 0xfffe;
    if(size>62)
    {
        size/=32;
        size--;
        size<<=10;
        size|=0x8000; //BL_SIZE=1
        SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
    } else {
        size<<=10;
        SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
    }
}

void EndpointRegister::IRQsetRxBuffer0(shmem_ptr addr, unsigned short size)
{
    int ep=EPR & USB_EP0R_EA;
    SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+0)=addr & 0xfffe;
    if(size>62)
    {
        size/=32;
        size--;
        size<<=10;
        size|=0x8000; //BL_SIZE=1
        SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
    } else {
        size<<=10;
        SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
    }
}

void EndpointRegister::IRQsetRxBuffer1(shmem_ptr addr, unsigned short size)
{
    IRQsetRxBuffer(addr,size);
}

unsigned short EndpointRegister::IRQgetReceivedBytes() const
{
    int ep=EPR & USB_EP0R_EA;
    return SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6) & 0x3ff;
}

unsigned short EndpointRegister::IRQgetReceivedBytes0() const
{
    int ep=EPR & USB_EP0R_EA;
    return SharedMemoryImpl::shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2) & 0x3ff;
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



//
// class USBperipheral
//

void USBperipheral::setAddress(unsigned short addr)
{
    USB->DADDR = addr | USB_DADDR_EF;
}

void USBperipheral::configureInterrupts()
{
    //Configure interrupts
    NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    NVIC_SetPriority(USB_HP_CAN1_TX_IRQn,3);//Higher priority (Max=0, min=15)
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn,4);//High priority (Max=0, min=15)
}

bool USBperipheral::enable()
{
    //Enable clock to USB peripheral
    #if __CM3_CMSIS_VERSION >= 0x010030 //CMSIS 1.3 changed variable names
    const int clock=SystemCoreClock;
    #else //__CM3_CMSIS_VERSION
    const int clock=SystemFrequency;
    #endif //__CM3_CMSIS_VERSION
    if(clock==72000000)
        RCC->CFGR &= ~RCC_CFGR_USBPRE; //Prescaler=1.5 (72MHz/1.5=48MHz)
    else if(clock==48000000)
        RCC->CFGR |= RCC_CFGR_USBPRE;  //Prescaler=1   (48MHz)
    else {
        return false;
    }
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    return true;
}

void USBperipheral::reset()
{
    USB->CNTR=USB_CNTR_FRES; //Clear PDWN, leave FRES asserted
    delayUs(1);  //Wait till USB analog circuitry stabilizes
    USB->CNTR=0; //Clear FRES too, USB peripheral active
    USB->ISTR=0; //Clear interrupt flags
    
    //First thing the host does is reset, so wait for that interrupt only
    USB->CNTR=USB_CNTR_RESETM;
}

void USBperipheral::disable()
{
    USB->DADDR=0;  //Clear EF bit
    USB->CNTR=USB_CNTR_PDWN | USB_CNTR_FRES;
    USB->ISTR=0; //Clear interrupt flags
    RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
}

void USBperipheral::ep0setTxStatus(RegisterStatus status)
{
    USB->endpoint[0].IRQsetTxStatus(status);
}

void USBperipheral::ep0setRxStatus(RegisterStatus status)
{
    USB->endpoint[0].IRQsetRxStatus(status);
}

unsigned short USBperipheral::ep0read(unsigned char *data, int size)
{
    unsigned short readBytes = USB->endpoint[0].IRQgetReceivedBytes();

    // if buffer size is not specified, read all bytes
    if (size <= 0) {
        size = readBytes;
    }

    SharedMemory::instance().copyBytesFrom(data,0,size);
    return readBytes;
}

void USBperipheral::ep0reset()
{
    // reset register
    USB->endpoint[0] = 0;
    // set register type
    USB->endpoint[0].IRQsetType(RegisterType::CONTROL);
    // set tx buffer
    USB->endpoint[0].IRQsetTxBuffer(SharedMemoryImpl::EP0TX_ADDR, EP0_SIZE);
    // set rx buffer
    USB->endpoint[0].IRQsetRxBuffer(SharedMemoryImpl::EP0RX_ADDR, EP0_SIZE);
}

void USBperipheral::ep0beginStatusTransaction()
{
    USB->endpoint[0].IRQsetEpKind();
}

void USBperipheral::ep0endStatusTransaction()
{
    USB->endpoint[0].IRQclearEpKind();
}

bool USBperipheral::ep0write(int size, const unsigned char *data)
{
    // push packet to TX FIFO if it is not a zero-length packet
    if (size > 0) {
        SharedMemory::instance().copyBytesTo(0,data,size);
    }

    USB->endpoint[0].IRQsetTxDataSize(size);

    return true;
}

} //namespace mxusb

#endif //_BOARD_STM32F103C8_BREAKOUT