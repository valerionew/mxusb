#include "new_hal_endpoint.h"

#ifdef MIOSIX_DEFINE_BOARD_NAME

#include "usb_tracer.h"
#include "usb_util.h"
#include "shared_memory.h"
#include "drivers/new_hal/new_hal_peripheral.h"
#include <algorithm>

using namespace std;

namespace mxusb {

//
// class EndpointImpl
//

void EndpointImpl::IRQdeconfigureAll()
{
    for(int i=1;i<NUM_ENDPOINTS;i++) EndpointImpl::get(i)->IRQdeconfigure(i);
    SharedMemory::instance().reset();
}

void EndpointImpl::IRQconfigureAll(const unsigned char *desc)
{
    const unsigned short wTotalLength=toShort(&desc[2]);
    unsigned short descSize=0;
    for(;;)
    {
        //Advance to next descriptor
        unsigned int sizeIncrement=desc[0];
        desc+=sizeIncrement;
        descSize+=sizeIncrement;
        if(descSize==wTotalLength) break;
        if(descSize>wTotalLength || sizeIncrement==0)
        {
            Tracer::IRQtrace(Ut::DESC_ERROR);
            return; //configuration descriptor is wrong
        }
        if(desc[1]!=Descriptor::ENDPOINT) continue;
        const unsigned char epNum=desc[2] & 0xf;
        EndpointImpl::get(epNum)->IRQconfigure(desc, NUM_ENDPOINTS);
    }
}

void EndpointImpl::IRQdeconfigure(int epNum)
{
    // TODO: add here endpoint deconfiguration

    EndpointBaseImpl::IRQdeconfigure(epNum);
}

bool EndpointImpl::write(const unsigned char *data, int size, int& written)
{
    unsigned char ep = IRQgetData().epNumber;
    
    written=min<unsigned int>(size,getSizeOfBuf());

    // TODO: add here configuration of endpoint write transaction

    // push packet to TX FIFO
    SharedMemory::instance().copyBytesTo(ep,data,written);

    Tracer::IRQtrace(Ut::IN_BUF_FILL,ep,written);
    return true;
}

bool EndpointImpl::read(unsigned char *data, int& readBytes)
{
    unsigned char ep = IRQgetData().epNumber;
    readBytes = 0;
    if(IRQgetData().enabledOut==0) return false;
    
    // TODO: add here implementation of endpoint read

    SharedMemory::instance().copyBytesFrom(data,ep,readBytes);
    
    Tracer::IRQtrace(Ut::OUT_BUF_READ,ep,readBytes);
    return true;
}

void EndpointImpl::IRQconfigureInterruptEndpoint(const unsigned char *desc)
{
    //Get endpoint data
    const unsigned char bEndpointAddress=desc[2];
    const unsigned char addr=bEndpointAddress & 0xf;
    const unsigned short wMaxPacketSize=toShort(&desc[4]);

    this->data.type=Descriptor::INTERRUPT;
    this->bufSize=wMaxPacketSize;

    // allocate RAM buffer
    const shmem_ptr ptr = SharedMemory::instance().allocate(addr, wMaxPacketSize);
    if(ptr==0 || wMaxPacketSize==0) {
        Tracer::IRQtrace(Ut::OUT_OF_SHMEM);
        return; //Out of memory, or wMaxPacketSize==0
    }

    if(bEndpointAddress & 0x80)
    {
        // IN endpoint

        // TODO: add here configuration and enabling of IN interrupt endpoint

        this->data.enabledIn=1;
    }
    else
    {
        // OUT endpoint

        // TODO: add here configuration and enabling of OUT interrupt endpoint

        this->data.enabledOut=1;
    }
}

void EndpointImpl::IRQconfigureBulkEndpoint(const unsigned char *desc)
{
    //Get endpoint data
    const unsigned char bEndpointAddress=desc[2];
    const unsigned char addr=bEndpointAddress & 0xf;
    const unsigned short wMaxPacketSize=toShort(&desc[4]);

    this->data.type=Descriptor::BULK;
    this->bufSize=wMaxPacketSize;
    this->bufCount=0;

    // allocate RAM buffer
    const shmem_ptr ptr = SharedMemory::instance().allocate(addr, wMaxPacketSize);
    if(ptr==0 || wMaxPacketSize==0) {
        Tracer::IRQtrace(Ut::OUT_OF_SHMEM);
        return; //Out of memory, or wMaxPacketSize==0
    }

    if(bEndpointAddress & 0x80)
    {
        // IN endpoint

        // TODO: add here configuration and enabling of IN bulk endpoint

        this->data.enabledIn=1;
    }
    else
    {
        // OUT endpoint

        // TODO: add here configuration and enabling of OUT bulk endpoint

        this->data.enabledOut=1;
    }
}

EndpointImpl EndpointImpl::endpoints[NUM_ENDPOINTS-1];
EndpointImpl EndpointImpl::invalidEp; //Invalid endpoint, always disabled

} //namespace mxusb

#endif //MIOSIX_DEFINE_BOARD_NAME