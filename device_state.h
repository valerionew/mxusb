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

#ifndef MXUSB_LIBRARY
#error "This is header is private, it can be used only within mxusb."
#error "If your code depends on a private header, it IS broken."
#endif //MXUSB_LIBRARY

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