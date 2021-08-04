#include "stm32f4xx_peripheral.h"

#ifdef _MIOSIX
#include "kernel/kernel.h"
using namespace miosix;
#else //_MIOSIX
#include "libraries/system.h"
#endif //_MIOSIX

#ifdef _BOARD_STM32F4DISCOVERY

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

extern void OTG_FS_WKUP_IRQHandler() __attribute__((naked));
void OTG_FS_WKUP_IRQHandler()
{
    #ifdef _MIOSIX
    //Since a context switch can happen within this interrupt handler, it is
    //necessary to save and restore context
    saveContext();
    asm volatile("bl _ZN5mxusb17USBWKUPirqHandlerEv");
    restoreContext();
    #else //_MIOSIX
    asm volatile("ldr r0, =_ZN5mxusb17USBWKUPirqHandlerEv\n\t"
                 "bx  r0                                 \n\t");
    #endif //_MIOSIX
}


namespace mxusb {

//
// class EndpointRegister
//

void EndpointRegister::IRQsetType(RegisterType type)
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits, so not to toggle any of them
    // reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
    //        USB_EP0R_STAT_TX);
    // //Avoid clearing an interrupt flag because of a read-modify-write
    // reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    // reg &= ~USB_EP0R_EP_TYPE;
    // reg |= type;
    // EPR=reg;
}

void EndpointRegister::IRQsetTxStatus(RegisterStatus status)
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits except STAT_TX, since we need to toggle STAT_TX
    // reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX);
    // //Avoid clearing an interrupt flag because of a read-modify-write
    // reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    // if(status & (1<<0)) reg ^=USB_EP0R_STAT_TX_0;
    // if(status & (1<<1)) reg ^=USB_EP0R_STAT_TX_1;
    // EPR=reg;
}

RegisterStatus EndpointRegister::IRQgetTxStatus() const
{
    // return static_cast<RegisterStatus>((EPR>>4) & 0x3);
    return RegisterStatus::DISABLED;
}

void EndpointRegister::IRQsetRxStatus(RegisterStatus status)
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits except STAT_RX, since we need to toggle STAT_RX
    // reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_TX);
    // //Avoid clearing an interrupt flag because of a read-modify-write
    // reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    // if(status & (1<<0)) reg ^=USB_EP0R_STAT_RX_0;
    // if(status & (1<<1)) reg ^=USB_EP0R_STAT_RX_1;
    // EPR=reg;
}

RegisterStatus EndpointRegister::IRQgetRxStatus() const
{
    // return static_cast<RegisterStatus>((EPR>>12) & 0x3);
    return RegisterStatus::DISABLED;
}

void EndpointRegister::IRQsetTxBuffer(shmem_ptr addr, unsigned short size)
{
    // int ep=EPR & USB_EP0R_EA;
    // SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+0)=addr & 0xfffe;
    // SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
}

void EndpointRegister::IRQsetTxBuffer0(shmem_ptr addr, unsigned short size)
{
    // IRQsetTxBuffer(addr,size);
}

void EndpointRegister::IRQsetTxBuffer1(shmem_ptr addr, unsigned short size)
{
    // int ep=EPR & USB_EP0R_EA;
    // SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+4)=addr & 0xfffe;
    // SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
}

void EndpointRegister::IRQsetTxDataSize(unsigned short size)
{
    // int ep=EPR & USB_EP0R_EA;
    // SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
}

void EndpointRegister::IRQsetTxDataSize0(unsigned short size)
{
    // IRQsetTxDataSize(size);
}

void EndpointRegister::IRQsetTxDataSize1(unsigned short size)
{
    // int ep=EPR & USB_EP0R_EA;
    // SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
}

void EndpointRegister::IRQsetRxBuffer(shmem_ptr addr, unsigned short size)
{
    // int ep=EPR & USB_EP0R_EA;
    // SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+4)=addr & 0xfffe;
    // if(size>62)
    // {
    //     size/=32;
    //     size--;
    //     size<<=10;
    //     size|=0x8000; //BL_SIZE=1
    //     SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
    // } else {
    //     size<<=10;
    //     SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6)=size;
    // }
}

void EndpointRegister::IRQsetRxBuffer0(shmem_ptr addr, unsigned short size)
{
    // int ep=EPR & USB_EP0R_EA;
    // SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+0)=addr & 0xfffe;
    // if(size>62)
    // {
    //     size/=32;
    //     size--;
    //     size<<=10;
    //     size|=0x8000; //BL_SIZE=1
    //     SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
    // } else {
    //     size<<=10;
    //     SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2)=size;
    // }
}

void EndpointRegister::IRQsetRxBuffer1(shmem_ptr addr, unsigned short size)
{
    // IRQsetRxBuffer(addr,size);
}

unsigned short EndpointRegister::IRQgetReceivedBytes() const
{
    // int ep=EPR & USB_EP0R_EA;
    // return SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+6) & 0x3ff;
    return 0;
}

unsigned short EndpointRegister::IRQgetReceivedBytes0() const
{
    // int ep=EPR & USB_EP0R_EA;
    // return SharedMemory::instance().shortAt(SharedMemoryImpl::BTABLE_ADDR+8*ep+2) & 0x3ff;
    return 0;
}

unsigned short EndpointRegister::IRQgetReceivedBytes1() const
{
    // return IRQgetReceivedBytes();
    return 0;
}

void EndpointRegister::IRQclearTxInterruptFlag()
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits, so not to toggle any of them.
    // //Additionally, clear CTR_TX
    // reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
    //         USB_EP0R_STAT_TX | USB_EP0R_CTR_TX);
    // //Explicitly set CTR_RX to avoid clearing it due to the read-modify-write op
    // reg |= USB_EP0R_CTR_RX;
    // EPR=reg;
}

void EndpointRegister::IRQclearRxInterruptFlag()
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits, so not to toggle any of them.
    // //Additionally, clear CTR_RX
    // reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
    //         USB_EP0R_STAT_TX | USB_EP0R_CTR_RX);
    // //Explicitly set CTR_TX to avoid clearing it due to the read-modify-write op
    // reg |= USB_EP0R_CTR_TX;
    // EPR=reg;
}

void EndpointRegister::IRQsetEpKind()
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits, so not to toggle any of them
    // reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
    //        USB_EP0R_STAT_TX);
    // //Avoid clearing an interrupt flag because of a read-modify-write
    // reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    // reg |= USB_EP0R_EP_KIND;
    // EPR=reg;
}

void EndpointRegister::IRQclearEpKind()
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits, so not to toggle any of them
    // reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX |
    //        USB_EP0R_STAT_TX);
    // //Avoid clearing an interrupt flag because of a read-modify-write
    // reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    // reg &= ~USB_EP0R_EP_KIND;
    // EPR=reg;
}

void EndpointRegister::IRQsetDtogTx(bool value)
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits except DTOG_TX, since we need to toggle it
    // reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_STAT_RX | USB_EP0R_STAT_TX);
    // //Avoid clearing an interrupt flag because of a read-modify-write
    // reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    // if(value) reg ^=USB_EP0R_DTOG_TX;
    // EPR=reg;
}

void EndpointRegister::IRQtoggleDtogTx()
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits except DTOG_TX, since we need to toggle it
    // reg &= ~(USB_EP0R_DTOG_RX | USB_EP0R_STAT_RX | USB_EP0R_STAT_TX);
    // //Avoid clearing an interrupt flag because of a read-modify-write
    // reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX | USB_EP0R_DTOG_TX;
    // EPR=reg;
}

bool EndpointRegister::IRQgetDtogTx() const
{
    // return (EPR & USB_EP0R_DTOG_TX)!=0;
    return false;
}

void EndpointRegister::IRQsetDtogRx(bool value)
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits except DTOG_RX, since we need to toggle it
    // reg &= ~(USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX | USB_EP0R_STAT_TX);
    // //Avoid clearing an interrupt flag because of a read-modify-write
    // reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX;
    // if(value) reg ^=USB_EP0R_DTOG_RX;
    // EPR=reg;
}

void EndpointRegister::IRQtoggleDtogRx()
{
    // unsigned short reg=EPR;
    // //Clear all toggle bits except DTOG_RX, since we need to toggle it
    // reg &= ~(USB_EP0R_DTOG_TX | USB_EP0R_STAT_RX | USB_EP0R_STAT_TX);
    // //Avoid clearing an interrupt flag because of a read-modify-write
    // reg |= USB_EP0R_CTR_RX | USB_EP0R_CTR_TX | USB_EP0R_DTOG_RX;
    // EPR=reg;
}

bool EndpointRegister::IRQgetDtogRx() const
{
    // return (EPR & USB_EP0R_DTOG_RX)!=0;
    return false;
}



//
// class USBperipheral
//

void USBperipheral::setAddress(unsigned short addr)
{
    // USB->DADDR = addr;
}

void USBperipheral::configureInterrupts()
{
    //Configure interrupts
    NVIC_EnableIRQ(OTG_FS_IRQn);
    NVIC_SetPriority(OTG_FS_IRQn,3);//Higher priority (Max=0, min=15)
    NVIC_EnableIRQ(OTG_FS_WKUP_IRQn);
    NVIC_SetPriority(OTG_FS_WKUP_IRQn,4);//Higher priority (Max=0, min=15)
}

bool USBperipheral::enable()
{
    // F1 CODE
    //Enable clock to USB peripheral
    // #if __CM3_CMSIS_VERSION >= 0x010030 //CMSIS 1.3 changed variable names
    // const int clock=SystemCoreClock;
    // #else //__CM3_CMSIS_VERSION
    // const int clock=SystemFrequency;
    // #endif //__CM3_CMSIS_VERSION
    // if(clock==72000000)
    //     RCC->CFGR &= ~RCC_CFGR_USBPRE; //Prescaler=1.5 (72MHz/1.5=48MHz)
    // else if(clock==48000000)
    //     RCC->CFGR |= RCC_CFGR_USBPRE;  //Prescaler=1   (48MHz)
    // else {
    //     return false;
    // }
    // RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    // return true;

    iprintf("Inside enable\n");
    power_on();
    iprintf("Power on finished\n");

    iprintf("Periph base: %lux\n", USB_OTG_FS_PERIPH_BASE);
    iprintf("Device base: %lux\n", USB_OTG_DEVICE_BASE);
    iprintf("sum: %lux\n", USB_OTG_FS_PERIPH_BASE+USB_OTG_DEVICE_BASE);
    iprintf("access: %lu\n", USB_OTG_DEVICE);
    iprintf("access: %lu\n", &(USB_OTG_DEVICE->DCFG));
    
    core_initialization();
    iprintf("Core initialization finished\n");

    // Current mode of operation == device
    if ((USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_CMOD) == 0)
    {
        device_initialization();
        iprintf("Device initialization finished\n");
        return true;
        // After that, the endpoint initialization must be done at the USB reset signal.
    }
    else // Current mode == host
    {
        //TODO return a message saying "Host mode not supported bu mxusb". This requires a change in the return value of this method.
        return false;
    }
}

void USBperipheral::power_on()
{
    // Enable clock to OTG FS peripheral
    RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;

    // Reset the power and clock gating control register (I do this to avoid spurious behaviour)
    // ST did not create a struct for this register and so it has to be accessed in a raw way
    *((uint32_t*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE)) = 0;
}

void USBperipheral::core_initialization()
{
    // FIELDS IN OTG_FS_GAHBCFG REGISTER
    // Global interrupt mask bit GINTMSK = 1
    USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;
    // Only TxFIFO empty level is programmed, as the Periodic TxFIFO empty level is only accessed in host mode
    USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_TXFELVL;

    // FIELDS IN OTG_FS_GUSBCFG
    // Peripheral only mode forced and then wait for the change to take effect (it takes at least 25 ms).
    // I think it is better to proceed only in a known state. An other solution could have been to avoid checking if the device mode is active, as we already forced it.
    USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;
    while ((USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_CMOD) != 0) ;

    // HNP and SRP disabled (only "peripheral only" supported)
    USB_OTG_FS->GUSBCFG &= ~(USB_OTG_GUSBCFG_HNPCAP | USB_OTG_GUSBCFG_SRPCAP);
    // FS timeout calibration field and USB turnaround
    // FIXME  comment -> I didn't understand how to deal with them, I copied the values from another programming model
    USB_OTG_FS->GUSBCFG |= 5; // TOCAL value = 5
    USB_OTG_FS->GUSBCFG |= 15<<10; // TRDT value = 15

    // FIELDS IN OTG_FS_GINTMSK
    // OTG interrupt mask and mode mismatch interrupt mask
    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_OTGINT | USB_OTG_GINTMSK_MMISM;
}

void USBperipheral::device_initialization()
{
    // FIELDS IN OTG_FS_DCFG
    // Device speed set at full speed and non-zero-length status  OUT handshake
    USB_OTG_DEVICE->DCFG |= USB_OTG_DCFG_DSPD | USB_OTG_DCFG_NZLSOHSK;
    
    // Clear pending interrupts
    USB_OTG_FS->GINTSTS = 0xFFFFFFFF;

    // FIELDS IN OTG_FS_GINTMSK
    // Unmask interrupts
    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_ESUSPM
                            | USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_SOFM | USB_OTG_GINTMSK_RXFLVLM;

    // Enable the VBUS sensing device
    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBUSBSEN;
    // Switch on full-speed transceiver module of PHY.
    // ST switch on power using the bit "power down", very weird and tricky...
    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;
}

void USBperipheral::reset()
{
    // USB->CNTR=USB_CNTR_FRES; //Clear PDWN, leave FRES asserted
    // delayUs(1);  //Wait till USB analog circuitry stabilizes
    // USB->CNTR=0; //Clear FRES too, USB peripheral active
    // USB->ISTR=0; //Clear interrupt flags
    
    // //First thing the host does is reset, so wait for that interrupt only
    // USB->CNTR=USB_CNTR_RESETM;
}

void USBperipheral::IRQUSBReset()
{
    //Set NAK bit for all out endpoints
    for (int i = 0; i<NUM_ENDPOINTS; i++)
    {
        EP_OUT(i)->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
    }

    //Unmask interrupt bits
    USB_OTG_DEVICE->DAINTMSK |= 1<<0;   // control 0 IN endpoint
    USB_OTG_DEVICE->DAINTMSK |= 1<<16;  // control 0 OUT endpoint
    USB_OTG_DEVICE->DOEPMSK |= USB_OTG_DOEPMSK_STUPM;
    USB_OTG_DEVICE->DOEPMSK |= USB_OTG_DOEPMSK_XFRCM;
    USB_OTG_DEVICE->DIEPMSK |= USB_OTG_DIEPMSK_XFRCM;
    USB_OTG_DEVICE->DIEPMSK |= USB_OTG_DIEPMSK_TOM; // STM typed TOC instead of TOM in the documentation: be careful

    // TODO -> Setup data FIFO RAM

    //EP0 able to receive 3 back-to-back SETUP packages
    EP_OUT(0)->DOEPTSIZ |= USB_OTG_DOEPTSIZ_STUPCNT;
}

void USBperipheral::disable()
{
    // USB->DADDR=0;  //Clear EF bit
    // USB->CNTR=USB_CNTR_PDWN | USB_CNTR_FRES;
    // USB->ISTR=0; //Clear interrupt flags
    // RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
}

void USBperipheral::ep0setTxStatus(RegisterStatus status)
{
    // USB->endpoint[0].IRQsetTxStatus(status);
}

void USBperipheral::ep0setRxStatus(RegisterStatus status)
{
    // USB->endpoint[0].IRQsetRxStatus(status);
}

unsigned short USBperipheral::ep0getReceivedBytes()
{
    // return USB->endpoint[0].IRQgetReceivedBytes();
    return 0;
}

void USBperipheral::ep0reset()
{
    // USB->endpoint[0] = 0;
}

void USBperipheral::ep0beginStatusTransaction()
{
    // USB->endpoint[0].IRQsetEpKind();
}

void USBperipheral::ep0endStatusTransaction()
{
    // USB->endpoint[0].IRQclearEpKind();
}

void USBperipheral::ep0setTxDataSize(unsigned short size)
{
    // USB->endpoint[0].IRQsetTxDataSize(size);
}

void USBperipheral::ep0setType(RegisterType type)
{
    // USB->endpoint[0].IRQsetType(type);
}

void USBperipheral::ep0setTxBuffer()
{
    // USB->endpoint[0].IRQsetTxBuffer(SharedMemory::instance().getEP0TxAddr(), EP0_SIZE);
}

void USBperipheral::ep0setRxBuffer()
{
    // USB->endpoint[0].IRQsetRxBuffer(SharedMemory::instance().getEP0RxAddr(), EP0_SIZE);
}

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY