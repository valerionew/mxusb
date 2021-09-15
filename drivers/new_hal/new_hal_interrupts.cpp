#include "drivers/new_hal/new_hal_peripheral.h"
#include "drivers/new_hal/new_hal_endpoint.h"
#include "drivers/new_hal/new_hal_memory.h"
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


#ifdef MIOSIX_DEFINE_BOARD_NAME

namespace mxusb {

/**
 * \internal
 * Handles USB device RESET
 */
static void IRQhandleReset()
{
    Tracer::IRQtrace(Ut::DEVICE_RESET);

    // deconfigure all endpoints
    for (int i = 0; i < NUM_ENDPOINTS; i++) {
        EndpointImpl::get(i)->IRQdeconfigure(i);
    }

    // TODO: add here implementation of reset interrupt handler

    Tracer::log(">>[int] end reset");
}

/**
 * \internal
 * Actual low priority interrupt handler.
 */
void USBirqHandler() __attribute__ ((noinline));
void USBirqHandler()
{
    //unsigned long status = INTERRUPT_FLAGS_REGISTER;
    Callbacks *callbacks = Callbacks::IRQgetCallbacks();

    /*if (status & USB_RESET_FLAG)
    {
        // reset event
        IRQhandleReset();
        callbacks->IRQreset();
        return; // reset causes all interrupt flags to be ignored
    }
    else if (status & USB_OUT_TRANSACTION_COMPLETED_FLAG)
    {
        // TODO: add here implementation of OUT transaction completed interrupt handler
    }
    else if (status & USB_IN_TRANSACTION_COMPLETED_FLAG)
    {
        // TODO: add here implementation of IN transaction completed interrupt handler
    }
    else if (status & USB_SUSPEND_FLAG)
    {
        // TODO: add here implementation of suspend interrupt handler

        Tracer::IRQtrace(Ut::SUSPEND_REQUEST);
        DeviceStateImpl::IRQsetSuspended(true);
        //If device is configured, deconfigure all endpoints. This in turn will
        //wake the threads waiting to write/read on endpoints
        if(USBdevice::IRQgetState()==USBdevice::CONFIGURED)
            EndpointImpl::IRQdeconfigureAll();
        callbacks->IRQsuspend();
    }
    else if (status & USB_WAKEUP_FLAG)
    {
        // TODO: add here implementation of wakeup interrupt handler

        Tracer::IRQtrace(Ut::RESUME_REQUEST);
        DeviceStateImpl::IRQsetSuspended(false);
        callbacks->IRQresume();
        //Reconfigure all previously deconfigured endpoints
        unsigned char conf=USBdevice::IRQgetConfiguration();
        if(conf!=0)
            EndpointImpl::IRQconfigureAll(DefCtrlPipe::IRQgetConfigDesc(conf));
    }*/
}

} //namespace mxusb

#endif //MIOSIX_DEFINE_BOARD_NAME