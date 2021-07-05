#ifndef DEVICE_STATE_H
#define DEVICE_STATE_H

#include "usb.h"

#ifdef _MIOSIX
#include "kernel/kernel.h"
#include "kernel/sync.h"
#include "kernel/scheduler/scheduler.h"
#endif //_MIOSIX

namespace mxusb {

/**
 * \internal
 * Implemenation class for DeviceState facade class.
 * It contains all what should be accessible from within the mxgui library,
 * but not accessible by user code.
 */
class DeviceStateImpl
{
public:
    /**
     * \return current device state.
     * Can be called both when interrupts are disabled or not.
     */
    static USBdevice::State getState() { return state; }

    /**
     * \internal
     * Set device state. If newstate!=oldstate calls the state change callback
     * \param s new state
     */
    static void IRQsetState(USBdevice::State s);

    /**
     * \return The configuration that the USB host has selected.
     * Zero means unconfigured.
     * Can be called both when interrupts are disabled or not.
     */
    static unsigned char getConfiguration() { return configuration; }

    /**
     * \internal
     * Set device configuration. Also calls configuration change callabck.
     * Note that the callback is called even if oldconfig==config.
     * Can be called only when getState()==CONFIGURED otherwise it does nothing
     * \param c configuration
     */
    static void IRQsetConfiguration(unsigned char c)
    {
        //Important: configuration is assigned before the callback is called
        //because if the callback calls IRQgetConfiguration() it must see the
        //new configuration number
        configuration=c;
        Callbacks::IRQgetCallbacks()->IRQconfigurationChanged();
    }

    /**
     * Wait until device is configured. If device is already configured, it
     * does nothing.
     */
    static void waitUntilConfigured()
    {
        #ifdef _MIOSIX
        miosix::InterruptDisableLock dLock;
        for(;;)
        {
            if(state==USBdevice::CONFIGURED) return;
            configWaiting=miosix::Thread::IRQgetCurrentThread();
            configWaiting->IRQwait();
            {
                miosix::InterruptEnableLock eLock(dLock);
                miosix::Thread::yield(); //The wait becomes effective
            }
        }
        #else //_MIOSIX
        while(state!=USBdevice::CONFIGURED) ;
        #endif //_MIOSIX
    }

    /**
     * Set suspend state
     * \param susp true if suspended
     */
    static void IRQsetSuspended(bool susp) { suspended=susp; }

    /**
     * \return true if suspended
     */
    static bool isSuspended() { return suspended; }

private:
    DeviceStateImpl();

    static volatile USBdevice::State state; ///< Current device state
    static volatile unsigned char configuration; ///< Current device config
    static volatile bool suspended; ///< True if suspended
    #ifdef _MIOSIX
    static miosix::Thread *configWaiting;
    #endif //_MIOSIX
};

} //namespace mxusb

#endif //DEVICE_STATE_H