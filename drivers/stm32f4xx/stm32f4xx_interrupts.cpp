#include "drivers/stm32f4xx/stm32f4xx_peripheral.h"
#include "drivers/stm32f4xx/stm32f4xx_endpoint.h"
#include "drivers/stm32f4xx/stm32f4xx_memory.h"
#include "def_ctrl_pipe.h"
#include "usb_tracer.h"
#include <config/usb_config.h>

#ifdef _MIOSIX
#include "interfaces/delays.h"
#include "kernel/kernel.h"
#include "interfaces-impl/portability_impl.h"
using namespace miosix;
#else //_MIOSIX
#include "libraries/system.h"
#endif //_MIOSIX


#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

/**
 * \internal
 * Handles USB device RESET
 */
static void IRQhandleReset()
{
    Tracer::IRQtrace(Ut::DEVICE_RESET);

    USB_OTG_FS->GINTSTS = 0xFFFFFFFF; //When the device is reset, clear all pending interrupts

    // deconfigure all endpoints
    for (int i = 0; i < NUM_ENDPOINTS; i++) {
        EndpointImpl::get(i)->IRQdeconfigure(i);
    }

    SharedMemory::instance().reset();

    //After a reset device address is zero, enable transaction handling
    USB_OTG_DEVICE->DCFG &= ~(USB_OTG_DCFG_DAD);

    // Unmask RX and TX interrupts on EP0
    USB_OTG_DEVICE->DAINTMSK |= 0x00010001;
    USB_OTG_DEVICE->DOEPMSK |= (USB_OTG_DOEPMSK_XFRCM | USB_OTG_DOEPMSK_STUPM);
    USB_OTG_DEVICE->DIEPMSK |= (USB_OTG_DIEPMSK_XFRCM);

    //Set STUPCNT=3 to receive up to 3 back-to-back SETUP packets
    EP_OUT(0)->DOEPTSIZ = EP0_SIZE*3 | USB_OTG_DOEPTSIZ_PKTCNT | USB_OTG_DOEPTSIZ_STUPCNT;

    USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_RXFFLSH;
    while(((USB_OTG_FS->GRSTCTL) & (USB_OTG_GRSTCTL_RXFFLSH)) != 0) ;

    USB_OTG_FS->GRSTCTL &= ~USB_OTG_GRSTCTL_TXFNUM;
    USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_TXFFLSH;
    while(((USB_OTG_FS->GRSTCTL) & (USB_OTG_GRSTCTL_TXFFLSH)) != 0) ;

    Tracer::log(">>[int] end reset");
}

/**
 * \internal
 * Handles USB device ENUM DONE
 */
static void IRQhandleEnumDone()
{
    Tracer::log(">>[int] enum done");

    USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_ENUMDNE; //Clear interrupt flag

    DefCtrlPipe::IRQdefaultStatus();

    //Device is now in the default address state
    DeviceStateImpl::IRQsetState(USBdevice::DEFAULT);

    Tracer::log(">>[int] end enum done");
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
        // disable rx status queue level interrupt
        USB_OTG_FS->GINTMSK &= ~USB_OTG_GINTMSK_RXFLVLM;

        unsigned long pop = USB_OTG_FS->GRXSTSR;
        unsigned char epNum = pop & USB_OTG_GRXSTSP_EPNUM;
        switch ((pop & USB_OTG_GRXSTSP_PKTSTS) >> 17) {
            case 0x02: // OUT data packet received
            {
                Tracer::log(">>[int] rxflvl: out data packet");
                if (epNum == 0) {
                    // handle OUT data packet on ep0
                    DefCtrlPipe::IRQstatusNak();
                    DefCtrlPipe::IRQout();
                    DefCtrlPipe::IRQrestoreStatus();
                }
                else {
                    // handle OUT data packet on other eps
                    EndpointImpl *epi = EndpointImpl::IRQget(epNum);
                    callbacks->IRQendpoint(epNum,Endpoint::OUT);
                    epi->IRQwakeWaitingThreadOnOutEndpoint();
                }

                EP_OUT(epNum)->DOEPINT = USB_OTG_DOEPINT_XFRC; // Clear interrupt flag
                break;
            }
            case 0x03: // OUT transfer completed
                break;
            case 0x04: // SETUP transaction completed
                Tracer::log(">>[int] rxflvl: out/setup completed");
                EP_OUT(epNum)->DOEPCTL |= USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA;
                break;
            case 0x06: // SETUP data packet received
                Tracer::log(">>[int] rxflvl: setup packet");
                DefCtrlPipe::IRQstatusNak();
                DefCtrlPipe::IRQsetup();
                DefCtrlPipe::IRQrestoreStatus();

                EP_OUT(epNum)->DOEPINT = USB_OTG_DOEPINT_STUP; // Clear interrupt flag
                break;
            default:
                // pop GRXSTSP
                USB_OTG_FS->GRXSTSP;
                break;
        }

        // enable rx status queue level interrupt
        USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;
    }
    else if (status & USB_OTG_GINTSTS_IEPINT)
    {
        uint32_t daint = USB_OTG_DEVICE->DAINT & 0xFFFF;
        int epNum = 0;
        // iterate over all endpoints because a single interrupt
        // can be generated for many endpoints
        while (daint)
        {
            if (daint & 0x1) {
                if (EP_IN(epNum)->DIEPINT & USB_OTG_DIEPINT_XFRC) {
                    EP_IN(epNum)->DIEPINT = USB_OTG_DIEPINT_XFRC; // Clear interrupt flag

                    Tracer::log(">>[int] iepint: in data packet");

                    if (epNum == 0) {
                        // handle IN data packet on ep0
                        DefCtrlPipe::IRQstatusNak();
                        DefCtrlPipe::IRQin();
                        DefCtrlPipe::IRQrestoreStatus();
                    }
                    else {
                        // handle IN data packet on other eps
                        EndpointImpl *epi = EndpointImpl::IRQget(epNum);
                        callbacks->IRQendpoint(epNum,Endpoint::IN);
                        epi->IRQwakeWaitingThreadOnInEndpoint();
                    }
                }
            }
            epNum++;
            daint >>= 1;
        }
    }
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

        // power down peripheral during suspended state
        *PCGCCTL |= (USB_OTG_PCGCCTL_STOPCLK | USB_OTG_PCGCCTL_GATECLK);
    }
    else if (status & USB_OTG_GINTSTS_WKUINT)
    {
        // resume peripheral after suspended state
        *PCGCCTL = 0;

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