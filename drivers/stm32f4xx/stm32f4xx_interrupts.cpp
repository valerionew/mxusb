//#include "stm32_usb_regs.h"
//FIXME: include should be implementation independent
#include "drivers/stm32f4xx/stm32f4xx_peripheral.h"
#include "def_ctrl_pipe.h"
#include "usb_tracer.h"
//FIXME: include should be implementation independent
#include "drivers/stm32f4xx/stm32f4xx_endpoint.h"
#include <config/usb_config.h>

#ifdef _MIOSIX
#include "interfaces/delays.h"
#include "kernel/kernel.h"
#include "interfaces-impl/portability_impl.h"
using namespace miosix;
#else //_MIOSIX
#include "libraries/system.h"
#endif //_MIOSIX

#include "drivers/stm32f4xx/stm32f4xx_memory.h"

#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

/**
 * \internal
 * Handles USB device RESET
 */
static void IRQhandleReset()
{
    Tracer::IRQtrace(Ut::DEVICE_RESET);

    //USB->DADDR=0;  //Disable transaction handling 
    USB_OTG_FS->GINTSTS = 0xFFFFFFFF; //When the device is reset, clear all pending interrupts

    //for(int i=1;i<NUM_ENDPOINTS;i++) EndpointImpl::get(i)->IRQdeconfigure(i);
    //Set NAK bit for all out endpoints
    // TODO: endpoints should be deconfiguted instead of simply nacked? check manual
    for (int i = 0; i < NUM_ENDPOINTS; i++) {
        EP_OUT(i)->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
    }

    SharedMemory::instance().reset();

    // //After a reset device address is zero, enable transaction handling
    // USB->DADDR=0 | USB_DADDR_EF;

    // //Enable more interrupt sources now that reset happened
    // USB->CNTR=USB_CNTR_CTRM | USB_CNTR_SUSPM | USB_CNTR_WKUPM | USB_CNTR_RESETM;

    // Unmask RX and TX interrupts on EP0
    USB_OTG_DEVICE->DAINTMSK |= 0x00010001;
    USB_OTG_DEVICE->DOEPMSK |= (USB_OTG_DOEPMSK_XFRCM | USB_OTG_DOEPMSK_STUPM);
    USB_OTG_DEVICE->DIEPMSK |= (USB_OTG_DIEPMSK_XFRCM | USB_OTG_DIEPMSK_TOM); // STM typed TOC instead of TOM in the documentation: be careful

    //Wait for the AHB bus to be ready, it takes some milliseconds
    // while((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0);

    //Set STUPCNT=3 to receive up to 3 back-to-back SETUP packets
    EP_OUT(0)->DOEPTSIZ = EP0_SIZE | USB_OTG_DOEPTSIZ_PKTCNT | USB_OTG_DOEPTSIZ_STUPCNT;
}

/**
 * \internal
 * Handles USB device ENUM DONE
 */
static void IRQhandleEnumDone()
{
    USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_ENUMDNE; //Clear interrupt flag

    DefCtrlPipe::IRQdefaultStatus();

    uint8_t size;
    if (EP0_SIZE == 8) size = 0x03;
    if (EP0_SIZE == 16) size = 0x02;
    if (EP0_SIZE == 32) size = 0x01;
    if (EP0_SIZE == 64) size = 0x00;

    EP_IN(0)->DIEPCTL = size | USB_OTG_DIEPCTL_EPENA;
    EP_OUT(0)->DOEPCTL = size | USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK; // FIXME: CNAK should be left?

    //Device is now in the default address state
    DeviceStateImpl::IRQsetState(USBdevice::DEFAULT);
}

/**
 * \internal
 * Actual low priority interrupt handler.
 */
void USBirqHandler() __attribute__ ((noinline));
void USBirqHandler()
{
    unsigned long status = USB_OTG_FS->GINTSTS;
    Callbacks *callbacks = Callbacks::IRQgetCallbacks();

    if (status & USB_OTG_GINTSTS_USBRST)
    {
        // reset event
        IRQhandleReset();
        return; // reset causes all interrupt flags to be ignored
    }
    else if (status & USB_OTG_GINTSTS_ENUMDNE)
    {
        // enumeration done event
        IRQhandleEnumDone();
        callbacks->IRQreset();
        return; // reset causes all interrupt flags to be ignored
    }
    else if (status & USB_OTG_GINTSTS_RXFLVL)
    {
        // TODO: check
        unsigned long pop = USB_OTG_FS->GRXSTSR;
        unsigned char epNum = pop & USB_OTG_GRXSTSP_EPNUM;
        switch ((pop & USB_OTG_GRXSTSP_PKTSTS) >> 17) {
            case 0x02: // OUT data packet received
            {
                EP_OUT(epNum)->DOEPINT = USB_OTG_DOEPINT_XFRC; // Clear interrupt flag

                if (epNum == 0) {
                    DefCtrlPipe::IRQstatusNak();
                    DefCtrlPipe::IRQout();
                    DefCtrlPipe::IRQrestoreStatus();
                }
                else {
                    EndpointImpl *epi = EndpointImpl::IRQget(epNum);
                    //NOTE: Increment buffer before the callback
                    // TODO: check why the buffer is inc and its behavior
                    epi->IRQincBufferCount();
                    callbacks->IRQendpoint(epNum,Endpoint::OUT);
                    epi->IRQwakeWaitingThreadOnOutEndpoint();
                }
                break;
            }
            case 0x03: // OUT transfer completed
                break;
            case 0x04: // SETUP transaction completed
                EP_OUT(epNum)->DOEPCTL |= USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA;
                break;
            case 0x06: // SETUP data packet received
                DefCtrlPipe::IRQstatusNak();
                DefCtrlPipe::IRQsetup();
                DefCtrlPipe::IRQrestoreStatus();
                break;
        }
    }
    else if (status & USB_OTG_GINTSTS_IEPINT)
    {
        // TODO: check
        unsigned char epNum;
        for (epNum = 0; epNum <= NUM_ENDPOINTS; epNum++) {
            if (USB_OTG_DEVICE->DAINT & (1 << epNum)) {
                break;
            }
        }
        
        EP_IN(epNum)->DIEPINT = USB_OTG_DIEPINT_XFRC; // Clear interrupt flag

        if (epNum == 0) {
            DefCtrlPipe::IRQstatusNak();
            DefCtrlPipe::IRQin();
            DefCtrlPipe::IRQrestoreStatus();
        }
        else {
            EndpointImpl *epi = EndpointImpl::IRQget(epNum);
            //NOTE: Decrement buffer before the callback
            // TODO: check why the buffer is dec and its behavior
            epi->IRQdecBufferCount();
            callbacks->IRQendpoint(epNum,Endpoint::IN);
            epi->IRQwakeWaitingThreadOnInEndpoint();
        }
    }
    /*else if (status & USB_OTG_GINTSTS_OEPINT)
    {

    }*/
    else if (status & USB_OTG_GINTSTS_USBSUSP)
    {
        USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_USBSUSP; //Clear interrupt flag
        Tracer::IRQtrace(Ut::SUSPEND_REQUEST);
        DeviceStateImpl::IRQsetSuspended(true);
        //If device is configured, deconfigure all endpoints. This in turn will
        //wake the threads waiting to write/read on endpoints
        if(USBdevice::IRQgetState()==USBdevice::CONFIGURED)
            EndpointImpl::IRQdeconfigureAll();
        callbacks->IRQsuspend();
    }

    // unsigned short flags=USB->ISTR;
    // Callbacks *callbacks=Callbacks::IRQgetCallbacks();
    // if(flags & USB_ISTR_RESET)
    // {
    //     IRQhandleReset();
    //     callbacks->IRQreset();
    //     return; //Reset causes all interrupt flags to be ignored
    // }
    // if(flags & USB_ISTR_SUSP)
    // {
    //     USB->ISTR= ~USB_ISTR_SUSP; //Clear interrupt flag
    //     USB->CNTR|=USB_CNTR_FSUSP;
    //     USB->CNTR|=USB_CNTR_LP_MODE;
    //     Tracer::IRQtrace(Ut::SUSPEND_REQUEST);
    //     DeviceStateImpl::IRQsetSuspended(true);
    //     //If device is configured, deconfigure all endpoints. This in turn will
    //     //wake the threads waiting to write/read on endpoints
    //     if(USBdevice::IRQgetState()==USBdevice::CONFIGURED)
    //         EndpointImpl::IRQdeconfigureAll();
    //     callbacks->IRQsuspend();
    // }
    // if(flags & USB_ISTR_WKUP)
    // {
    //     USB->ISTR= ~USB_ISTR_WKUP; //Clear interrupt flag
    //     USB->CNTR&= ~USB_CNTR_FSUSP;
    //     Tracer::IRQtrace(Ut::RESUME_REQUEST);
    //     DeviceStateImpl::IRQsetSuspended(false);
    //     callbacks->IRQresume();
    //     //Reconfigure all previously deconfigured endpoints
    //     unsigned char conf=USBdevice::IRQgetConfiguration();
    //     if(conf!=0)
    //         EndpointImpl::IRQconfigureAll(DefCtrlPipe::IRQgetConfigDesc(conf));
    // }
    // while(flags & USB_ISTR_CTR)
    // {
    //     int epNum=flags & USB_ISTR_EP_ID;
    //     unsigned short reg=USB->endpoint[epNum].get();
    //     if(epNum==0)
    //     {
    //         DefCtrlPipe::IRQstatusNak();
    //         //Transaction on endpoint zero
    //         if(reg & USB_EP0R_CTR_RX)
    //         {
    //             bool isSetupPacket=reg & USB_EP0R_SETUP;
    //             USB->endpoint[epNum].IRQclearRxInterruptFlag();
    //             if(isSetupPacket) DefCtrlPipe::IRQsetup();
    //             else DefCtrlPipe::IRQout();
    //         }

    //         if(reg & USB_EP0R_CTR_TX)
    //         {
    //             USB->endpoint[epNum].IRQclearTxInterruptFlag();
    //             DefCtrlPipe::IRQin();
    //         }
    //         DefCtrlPipe::IRQrestoreStatus();

    //     } else {
    //         //Transaction on other endpoints
    //         EndpointImpl *epi=EndpointImpl::IRQget(epNum);
    //         if(reg & USB_EP0R_CTR_RX)
    //         {
    //             USB->endpoint[epNum].IRQclearRxInterruptFlag();
    //             //NOTE: Increment buffer before the callabck
    //             epi->IRQincBufferCount();
    //             callbacks->IRQendpoint(epNum,Endpoint::OUT);
    //             epi->IRQwakeWaitingThreadOnOutEndpoint();
    //         }

    //         if(reg & USB_EP0R_CTR_TX)
    //         {
    //             USB->endpoint[epNum].IRQclearTxInterruptFlag();

    //             //NOTE: Decrement buffer before the callabck
    //             epi->IRQdecBufferCount();
    //             callbacks->IRQendpoint(epNum,Endpoint::IN);
    //             epi->IRQwakeWaitingThreadOnInEndpoint();
    //         }
    //     }
    //     //Read again the ISTR register so that if more endpoints have completed
    //     //a transaction, they are all serviced
    //     flags=USB->ISTR;
    // }
}

void USBWKUPirqHandler() __attribute__ ((noinline));
void USBWKUPirqHandler()
{
    unsigned long status = USB_OTG_FS->GINTSTS;
    Callbacks *callbacks = Callbacks::IRQgetCallbacks();

    if (status & USB_OTG_GINTSTS_WKUINT)
    {
        USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_WKUINT; //Clear interrupt flag
        Tracer::IRQtrace(Ut::RESUME_REQUEST);
        DeviceStateImpl::IRQsetSuspended(false);
        callbacks->IRQresume();
        //Reconfigure all previously deconfigured endpoints
        unsigned char conf=USBdevice::IRQgetConfiguration();
        if(conf!=0)
            EndpointImpl::IRQconfigureAll(DefCtrlPipe::IRQgetConfigDesc(conf));
    }
}

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY