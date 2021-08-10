#include "endpoint_base_impl.h"

#include "usb_tracer.h"
#include "usb_util.h"
#include "shared_memory.h"
#include <algorithm>

using namespace std;

namespace mxusb {

//
// class EndpointBaseImpl
//

void EndpointBaseImpl::IRQdeconfigure(int epNum)
{
    this->data.enabledIn=0;
    this->data.enabledOut=0;
    this->data.epNumber=epNum;
    this->IRQwakeWaitingThreadOnInEndpoint();
    this->IRQwakeWaitingThreadOnOutEndpoint();
}

void EndpointBaseImpl::IRQconfigure(const unsigned char *desc, int numEndpoints)
{
    const unsigned char bEndpointAddress=desc[2];
    unsigned char bmAttributes=desc[3];
    Tracer::IRQtrace(Ut::CONFIGURING_EP,bEndpointAddress,bmAttributes);

    const unsigned char addr=bEndpointAddress & 0xf;
    if(addr==0 || addr>numEndpoints-1 || addr!=this->data.epNumber)
    {
        Tracer::IRQtrace(Ut::DESC_ERROR);
        return; //Invalid ep #, or called with an endpoint with a wrong #
    }

    if((this->data.enabledIn==1  && ((bEndpointAddress & 0x80)==1)) ||
       (this->data.enabledOut==1 && ((bEndpointAddress & 0x80)==0)))
    {
        Tracer::IRQtrace(Ut::DESC_ERROR);
        return; //Trying to configure an ep direction twice
    }

    if((this->data.enabledIn==1 || this->data.enabledOut==1))
    {
        //We're trying to enable both sides of an endpoint.
        //This is only possible if both sides are of type INTERRUPT
        if((bmAttributes & Descriptor::TYPE_MASK)!=Descriptor::INTERRUPT ||
            this->data.type!=Descriptor::INTERRUPT)
        {
            Tracer::IRQtrace(Ut::DESC_ERROR);
            return; //Trying to enable both sides of a non INTERRUPT ep
        }
    }

    switch(bmAttributes & Descriptor::TYPE_MASK)
    {
        case Descriptor::INTERRUPT:
            IRQconfigureInterruptEndpoint(desc);
            break;
        case Descriptor::BULK:
            IRQconfigureBulkEndpoint(desc);
            break;
        case Descriptor::CONTROL:
        case Descriptor::ISOCHRONOUS:
            Tracer::IRQtrace(Ut::DESC_ERROR);
            return; //CONTROL and ISOCHRONOUS endpoints not supported
    }
}

} //namespace mxusb