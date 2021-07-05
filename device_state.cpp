
#include "device_state.h"
#include "usb_tracer.h"

namespace mxusb {

//
// class DeviceStateImpl
//

void DeviceStateImpl::IRQsetState(USBdevice::State s)
{
    if(state==s) return; //No real state change
    state=s;
    #ifdef _MIOSIX
    if(configWaiting!=0 && state==USBdevice::CONFIGURED)
    {
        configWaiting->IRQwakeup();
        configWaiting=0;
    }
    #endif //_MIOSIX
    Tracer::IRQtrace(Ut::DEVICE_STATE_CHANGE,state);
    Callbacks::IRQgetCallbacks()->IRQstateChanged();
}

volatile USBdevice::State DeviceStateImpl::state=USBdevice::DEFAULT;
volatile unsigned char DeviceStateImpl::configuration=0;
volatile bool DeviceStateImpl::suspended=false;
#ifdef _MIOSIX
miosix::Thread *DeviceStateImpl::configWaiting=0;
#endif //_MIOSIX

} //namespace mxusb
