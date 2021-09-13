#include "stm32f4xx_endpoint.h"

#ifdef _BOARD_STM32F4DISCOVERY

#include "usb_tracer.h"
#include "usb_util.h"
#include "shared_memory.h"
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
    volatile USB_OTG_INEndpointTypeDef *epi = EP_IN(epNum);
    volatile USB_OTG_OUTEndpointTypeDef *epo = EP_OUT(epNum);

    // disable interrupts
    USB_OTG_DEVICE->DAINTMSK &= ~(0x10001 << epNum);

    // deconfigure TX side
    epi->DIEPCTL &= ~(USB_OTG_DIEPCTL_USBAEP);
    if ((epNum != 0) && (epi->DIEPCTL & USB_OTG_DIEPCTL_EPENA)) {
        epi->DIEPCTL = USB_OTG_DIEPCTL_EPDIS | USB_OTG_DIEPCTL_SNAK;
    }
    // clear interrupts
    epi->DIEPINT = 0xFF;

    // deconfigure RX side
    epo->DOEPCTL &= ~(USB_OTG_DOEPCTL_USBAEP);
    if ((epNum != 0) && (epo->DOEPCTL & USB_OTG_DOEPCTL_EPENA)) {
        epo->DOEPCTL = USB_OTG_DOEPCTL_EPDIS | USB_OTG_DIEPCTL_SNAK;
    }
    // clear interrupts
    epo->DOEPINT = 0xFF;

    EndpointBaseImpl::IRQdeconfigure(epNum);
}

bool EndpointImpl::write(const unsigned char *data, int size, int& written)
{
    unsigned char ep = IRQgetData().epNumber;
    written=0;
    if (EP_IN(ep)-> DIEPCTL & USB_OTG_DIEPCTL_STALL) return false;

    if(IRQgetData().type==Descriptor::INTERRUPT)
    {
        //INTERRUPT
        if(!(EP_IN(ep)-> DIEPCTL & USB_OTG_DIEPCTL_NAKSTS)) return true; //No errors, just no data

    }

    written=min<unsigned int>(size,getSizeOfBuf());

    //No enough space in TX fifo
    uint32_t len = (written + 0x03) >> 2;
    if ((len) > EP_IN(ep)->DTXFSTS) return false;
    if (EP_IN(ep)->DIEPCTL & USB_OTG_DIEPCTL_EPENA) return false;
    
    // configure ep transaction in control registers
    EP_IN(ep)->DIEPTSIZ = 0;
    EP_IN(ep)->DIEPTSIZ = written | (1 << 19);
    EP_IN(ep)->DIEPCTL &= ~USB_OTG_DIEPCTL_STALL;
    EP_IN(ep)->DIEPCTL |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK;
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
    if (EP_OUT(ep)-> DOEPCTL & USB_OTG_DOEPCTL_STALL) return false;

    if(IRQgetData().type==Descriptor::INTERRUPT)
    {
        //INTERRUPT
        if(!(EP_OUT(ep)-> DOEPCTL & USB_OTG_DOEPCTL_NAKSTS)) return true; //No errors, just no data
    }

    // pop packet from RX FIFO
    readBytes = ((USB_OTG_FS->GRXSTSR & USB_OTG_GRXSTSP_BCNT) >> 4);
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

    if(bEndpointAddress & 0x80)
    {
        // IN endpoint

        // allocate TX FIFO
        const shmem_ptr ptr = SharedMemory::instance().allocate(addr, wMaxPacketSize);
        if(ptr==0 || wMaxPacketSize==0) {
            Tracer::IRQtrace(Ut::OUT_OF_SHMEM);
            return; //Out of memory, or wMaxPacketSize==0
        }

        // enable IN endpoint interrupt
        USB_OTG_DEVICE->DAINTMSK |= (0x0001 << addr);

        EP_IN(addr)->DIEPCTL = (RegisterType::INTERRUPT << 18) | wMaxPacketSize |
                                USB_OTG_DIEPCTL_SD0PID_SEVNFRM | USB_OTG_DIEPCTL_USBAEP |
                                (addr << 22) | USB_OTG_DIEPCTL_SNAK;

        this->data.enabledIn=1;
    }
    else
    {
        // OUT endpoint

        EP_OUT(addr)->DOEPCTL = (RegisterType::INTERRUPT << 18) | wMaxPacketSize |
                                USB_OTG_DOEPCTL_SD0PID_SEVNFRM | USB_OTG_DOEPCTL_USBAEP |
                                USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK;

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

    if(bEndpointAddress & 0x80)
    {
        // IN endpoint

        // allocate TX FIFO
        const shmem_ptr ptr = SharedMemory::instance().allocate(addr, wMaxPacketSize);
        if(ptr==0 || wMaxPacketSize==0) {
            Tracer::IRQtrace(Ut::OUT_OF_SHMEM);
            return; //Out of memory, or wMaxPacketSize==0
        }

        // enable IN endpoint interrupt
        USB_OTG_DEVICE->DAINTMSK |= (0x0001 << addr);

        EP_IN(addr)->DIEPCTL = (RegisterType::BULK << 18) | wMaxPacketSize |
                                USB_OTG_DIEPCTL_SD0PID_SEVNFRM | USB_OTG_DIEPCTL_USBAEP |
                                (addr << 22) | USB_OTG_DIEPCTL_SNAK;

        this->data.enabledIn=1;
    }
    else
    {
        // OUT endpoint

        EP_OUT(addr)->DOEPCTL = (RegisterType::BULK << 18) | wMaxPacketSize |
                                USB_OTG_DOEPCTL_SD0PID_SEVNFRM | USB_OTG_DOEPCTL_USBAEP |
                                USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK;

        this->data.enabledOut=1;
    }
}

EndpointImpl EndpointImpl::endpoints[NUM_ENDPOINTS-1];
EndpointImpl EndpointImpl::invalidEp; //Invalid endpoint, always disabled

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY