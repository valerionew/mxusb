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

#include "usb.h"
//#include "stm32_usb_regs.h"
//FIXME: include should be implementation independent
#include "drivers/stm32f1xx/stm32f1xx_peripheral.h"
#include "drivers/stm32f4xx/stm32f4xx_peripheral.h"
#include "shared_memory.h"
#include "def_ctrl_pipe.h"
#include "usb_tracer.h"
//FIXME: include should be implementation independent
#include "drivers/stm32f1xx/stm32f1xx_endpoint.h"
#include "drivers/stm32f4xx/stm32f4xx_endpoint.h"
//#include <config/usb_gpio.h>
//FIXME: include should be implementation independent
#include "drivers/stm32f1xx/stm32f1xx_gpio.h"
#include "drivers/stm32f4xx/stm32f4xx_gpio.h"
#include <config/usb_config.h>
#include <algorithm>

#if USB_CONFIG_VERSION != 100
#error Wrong usb_config.h version. You need to upgrade it.
#endif
#if USB_GPIO_VERSION != 100
#error Wrong usb_gpio.h version. You need to upgrade it.
#endif

#ifdef _MIOSIX
#include "interfaces/delays.h"
#include "kernel/kernel.h"
#include "interfaces-impl/portability_impl.h"
using namespace miosix;
#else //_MIOSIX
#include "libraries/system.h"
#endif //_MIOSIX

using namespace std;

namespace mxusb {

//
// class Endpoint
//

Endpoint Endpoint::get(unsigned char epNum)
{
    return Endpoint(EndpointImpl::get(epNum));
}

const int Endpoint::maxNumEndpoints()
{
    return NUM_ENDPOINTS;
}

void deconfigureAll()
{
    EndpointImpl::IRQdeconfigureAll();
}

void configureAll(const unsigned char *desc)
{
    EndpointImpl::IRQconfigureAll(desc);
}

bool Endpoint::isInSideEnabled() const
{
    return pImpl->getData().enabledIn==1;
}

bool Endpoint::isOutSideEnabled() const
{
    return pImpl->getData().enabledOut==1;
}

unsigned short Endpoint::inSize() const
{
    return pImpl->getSizeOfInBuf();
}

unsigned short Endpoint::outSize() const
{
    return pImpl->getSizeOfOutBuf();
}

bool Endpoint::write(const unsigned char *data, int size, int& written)
{
    written=0;
    #ifdef _MIOSIX
    InterruptDisableLock dLock;
    unsigned char initialConfig=DeviceStateImpl::getConfiguration();
    for(;;)
    {
        int partialWritten;
        bool result=IRQwrite(data,size,partialWritten);
        written+=partialWritten;
        data+=partialWritten;
        size-=partialWritten;
        if(size<=0) return true;
        if(result==false) return false; //Return error *after* updating written
        if(partialWritten==0)
        {
            Thread *self=Thread::IRQgetCurrentThread();
            pImpl->IRQsetWaitingThreadOnInEndpoint(self);
            self->IRQwait();
            {
                InterruptEnableLock eLock(dLock);
                Thread::yield(); //The wait becomes effective
            }
            //If configuration changet in the meantime, return error
            if(DeviceStateImpl::getConfiguration()!=initialConfig) return false;
        }
    }
    #else //_MIOSIX
    unsigned char initialConfig=DeviceStateImpl::getConfiguration();
    for(;;)
    {
        if(DeviceStateImpl::getConfiguration()!=initialConfig) return false;
        int partialWritten;
        __disable_irq();
        bool result=IRQwrite(data,size,partialWritten);
        __enable_irq();
        written+=partialWritten;
        data+=partialWritten;
        size-=partialWritten;
        if(size<=0) return true;
        if(result==false) return false; //Return error *after* updating written
        //if(partialWritten==0) __WFI();
        //FIXME: can't add a __WFI() here to minimize power consumption because
        //of a race condition: if the interrupt occurs after __enable_irq()
        //but before the __WFI() the program will lock. Miosix does not have
        //this race condition
    }
    #endif //_MIOSIX
}

bool Endpoint::read(unsigned char *data, int& readBytes)
{
    #ifdef _MIOSIX
    InterruptDisableLock dLock;
    unsigned char initialConfig=DeviceStateImpl::getConfiguration();
    for(;;)
    {
        if(IRQread(data,readBytes)==false) return false; //Error
        if(readBytes>0) return true; //Got data
        Thread *self=Thread::IRQgetCurrentThread();
        pImpl->IRQsetWaitingThreadOnOutEndpoint(self);
        self->IRQwait();
        {
            InterruptEnableLock eLock(dLock);
            Thread::yield(); //The wait becomes effective
        }
        //If configuration changet in the meantime, return error
        if(DeviceStateImpl::getConfiguration()!=initialConfig) return false;
    }
    #else //_MIOSIX
    unsigned char initialConfig=DeviceStateImpl::getConfiguration();
    for(;;)
    {
        if(DeviceStateImpl::getConfiguration()!=initialConfig) return false;
        __disable_irq();
        bool result=IRQread(data,readBytes);
        __enable_irq();
        if(result==false) return false; //Error
        if(readBytes>0) return true;    //Got data
        //__WFI();
        //FIXME: can't add a __WFI() here to minimize power consumption because
        //of a race condition: if the interrupt occurs after __enable_irq()
        //but before the __WFI() the program will lock. Miosix does not have
        //this race condition
    }
    #endif //_MIOSIX
}

bool Endpoint::IRQwrite(const unsigned char *data, int size, int& written)
{
    return pImpl->write(data, size, written);
}

bool Endpoint::IRQread(unsigned char *data, int& readBytes)
{
    return pImpl->read(data, readBytes);
}

//
// class Callbacks
//

void Callbacks::IRQendpoint(unsigned char epNum, Endpoint::Direction dir) {}

void Callbacks::IRQstateChanged() {}

void Callbacks::IRQconfigurationChanged() {}

void Callbacks::IRQsuspend() {}

void Callbacks::IRQresume() {}

void Callbacks::IRQreset() {}

Callbacks::~Callbacks() {}

void Callbacks::setCallbacks(Callbacks* callback)
{
    #ifdef _MIOSIX
    InterruptDisableLock dLock;
    #else //_MIOSIX
    __disable_irq();
    #endif //_MIOSIX

    if(callback==0) callbacks=&defaultCallbacks;
    else callbacks=callback;

    #ifndef _MIOSIX
    __enable_irq();
    #endif //_MIOSIX
}

Callbacks Callbacks::defaultCallbacks;
Callbacks *Callbacks::callbacks=&defaultCallbacks;

//
// class USBdevice
//

bool USBdevice::enable(const unsigned char *device,
            const unsigned char * const * configs,
            const unsigned char * const * strings,
            unsigned char numStrings)
{
    Tracer::init();
    if(DefCtrlPipe::registerAndValidateDescriptors(
            device,configs,strings,numStrings)==false) return false;

    #ifdef _MIOSIX
    {
    InterruptDisableLock dLock; //The easy way
    #else //_MIOSIX
    __disable_irq();
    #endif //_MIOSIX
    
    //Configure gpio for USB pullup
    USBgpio::init();
    
    //Enable clock to USB peripheral
    if (!USBperipheral::enable()) {
        return false;
    }

    //Connect pull-up to vcc
    USBgpio::enablePullup();

    // Reset USB peripheral
    USBperipheral::reset();
    
    DeviceStateImpl::IRQsetState(USBdevice::DEFAULT);

    USBperipheral::configureInterrupts();

    #ifdef _MIOSIX
    }
    #else //_MIOSIX
    __enable_irq();
    #endif //_MIOSIX
    return true;
}

void USBdevice::disable()
{
    #ifdef _MIOSIX
    {
    InterruptDisableLock dLock;
    #else //_MIOSIX
    __disable_irq();
    #endif //_MIOSIX

    USBgpio::disablePullup();
    for(int i=1;i<NUM_ENDPOINTS;i++) EndpointImpl::get(i)->IRQdeconfigure(i);
    SharedMemory::instance().reset();
    USBperipheral::disable();
    DeviceStateImpl::IRQsetState(USBdevice::DEFAULT);
    DeviceStateImpl::IRQsetConfiguration(0);
    #ifdef _MIOSIX
    }
    #else //_MIOSIX
    __enable_irq();
    #endif //_MIOSIX
    Tracer::shutdown();
}

USBdevice::State USBdevice::getState()
{
    return DeviceStateImpl::getState();
}

unsigned char USBdevice::getConfiguration()
{
    return DeviceStateImpl::getConfiguration();
}

void USBdevice::waitUntilConfigured()
{
    DeviceStateImpl::waitUntilConfigured();
}

bool USBdevice::isSuspended()
{
    return DeviceStateImpl::isSuspended();
}

} //namespace mxusb
