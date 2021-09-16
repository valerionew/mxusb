/***************************************************************************
 *   Copyright (C) 2011 by Terraneo Federico                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

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


namespace mxusb {

//
// class USBperipheral
//

void USBperipheralImpl::setAddress(unsigned short addr)
{
    USB_OTG_DEVICE->DCFG &= ~(USB_OTG_DCFG_DAD);
    USB_OTG_DEVICE->DCFG |= (addr << 4) & USB_OTG_DCFG_DAD;
}

void USBperipheralImpl::configureInterrupts()
{
    //Configure interrupts
    NVIC_EnableIRQ(OTG_FS_IRQn);
    NVIC_SetPriority(OTG_FS_IRQn,3);//Higher priority (Max=0, min=15)
}

bool USBperipheralImpl::enable()
{
    power_on();
    
    core_initialization();

    // Current mode of operation == device
    if ((USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_CMOD) == 0)
    {
        device_initialization();
        return true;
        // After that, the endpoint initialization must be done at the USB reset signal.
    }
    else // Current mode == host
    {
        return false;
    }
}

void USBperipheralImpl::power_on()
{
    // Enable clock to OTG FS peripheral
    RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;

    //Wait for the AHB bus to be ready, it takes some milliseconds
    while((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0);

    // Reset the power and clock gating control register (I do this to avoid spurious behaviour)
    // ST did not create a struct for this register and so it has to be accessed in a raw way
    *PCGCCTL = 0;
}

void USBperipheralImpl::core_initialization()
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
    USB_OTG_FS->GUSBCFG |= 5; // TOCAL value = 5
    USB_OTG_FS->GUSBCFG |= 15<<10; // TRDT value = 15

    // FIELDS IN OTG_FS_GINTMSK
    // OTG interrupt mask and mode mismatch interrupt mask
    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_OTGINT | USB_OTG_GINTMSK_MMISM;
}

void USBperipheralImpl::device_initialization()
{
    // FIELDS IN OTG_FS_DCFG
    // Device speed set at full speed and non-zero-length status  OUT handshake
    USB_OTG_DEVICE->DCFG |= USB_OTG_DCFG_DSPD;
    
    // Clear pending interrupts
    USB_OTG_FS->GINTSTS = 0xFFFFFFFF;

    // FIELDS IN OTG_FS_GINTMSK
    // Unmask interrupts
    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_IEPINT
                            | USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_WUIM | USB_OTG_GINTMSK_RXFLVLM;

    // Disable the VBUS sensing device
    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
    // Switch on full-speed transceiver module of PHY.
    // ST switch on power using the bit "power down", very weird and tricky...
    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;
}

void USBperipheralImpl::reset()
{
    // Clear pending interrupts
    USB_OTG_FS->GINTSTS = 0xFFFFFFFF;
}

void USBperipheralImpl::disable()
{
    USB_OTG_DEVICE->DCFG &= ~USB_OTG_DCFG_DAD; // reset device addr
    USB_OTG_FS->GINTSTS = 0xFFFFFFFF; // clear interrupts
    USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_PWRDWN; // power down peripheral
    RCC->AHB2ENR &= ~RCC_AHB2ENR_OTGFSEN; // disable clock to USB peripheral
}

void USBperipheralImpl::ep0setTxStatus(RegisterStatus status)
{
    if (status == RegisterStatus::STALL) {
        EP_IN(0)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;
    }
    else if (status == RegisterStatus::NAK) {
        EP_IN(0)->DIEPCTL |= USB_OTG_DIEPCTL_SNAK;
    }
    else if (status == RegisterStatus::VALID) {
        if (EP_IN(0)->DIEPCTL & USB_OTG_DIEPCTL_NAKSTS) {
            EP_IN(0)->DIEPCTL |= USB_OTG_DIEPCTL_CNAK;
        }
    }
}

void USBperipheralImpl::ep0setRxStatus(RegisterStatus status)
{
    if (status == RegisterStatus::STALL) {
        EP_OUT(0)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;
    }
    else if (status == RegisterStatus::NAK) {
        EP_OUT(0)->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
    }
    else if (status == RegisterStatus::VALID) {
        if (EP_OUT(0)->DOEPCTL & USB_OTG_DOEPCTL_NAKSTS) {
            EP_OUT(0)->DOEPCTL |= USB_OTG_DOEPCTL_CNAK;
        }
    }
}

unsigned short USBperipheralImpl::ep0read(unsigned char *data, int size)
{
    unsigned short readBytes = ((USB_OTG_FS->GRXSTSR & USB_OTG_GRXSTSP_BCNT) >> 4);

    // if buffer size is not specified, read all bytes
    if (size <= 0) {
        size = readBytes;
    }

    SharedMemory::instance().copyBytesFrom(data,0,size);
    return readBytes;
}

void USBperipheralImpl::ep0reset()
{
    uint8_t size;
    if (EP0_SIZE == 8) size = 0x03;
    if (EP0_SIZE == 16) size = 0x02;
    if (EP0_SIZE == 32) size = 0x01;
    if (EP0_SIZE == 64) size = 0x00;

    EP_IN(0)->DIEPCTL = size | USB_OTG_DIEPCTL_SNAK;
    EP_OUT(0)->DOEPCTL = size | USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK;

    // NOTES:
    // in F4 peripheral the type of EP0 is hardcoded to CONTROL
    // because this endpoint has a slightly different register wrt to other EPs

    // the EP0 tx and shared rx buffers are allocated and managed in F4 SharedMemoryImpl class
}

void USBperipheralImpl::ep0beginStatusTransaction()
{
    // NOTE: empty method
    // implemented in F1 driver
    // does not have an equivalent in F4 driver
}

void USBperipheralImpl::ep0endStatusTransaction()
{
    // NOTE: empty method
    // implemented in F1 driver
    // does not have an equivalent in F4 driver
}

bool USBperipheralImpl::ep0write(int size, const unsigned char *data)
{
    //No enough space in TX fifo
    uint32_t len = (size + 0x03) >> 2;
    if ((len) > EP_IN(0)->DTXFSTS) return false;
    
    // configure ep transaction in control registers
    EP_IN(0)->DIEPTSIZ = 0;
    EP_IN(0)->DIEPTSIZ = size | (1 << 19);
    EP_IN(0)->DIEPCTL |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK;

    // push packet to TX FIFO if it is not a zero-length packet
    if (size > 0) {
        SharedMemory::instance().copyBytesTo(0,data,size);
    }
    
    return true;
}

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY