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
    // USB->endpoint[epNum]=epNum;
    EndpointBaseImpl::IRQdeconfigure(epNum);
}

bool EndpointImpl::write(const unsigned char *data, int size, int& written)
{
    // written=0;
    // if(IRQgetData().enabledIn==0) return false;
    // EndpointRegister& epr=USB->endpoint[IRQgetData().epNumber];
    // RegisterStatus stat=epr.IRQgetTxStatus();
    // if(stat==RegisterStatus::STALL) return false;

    // if(IRQgetData().type==Descriptor::INTERRUPT)
    // {
    //     //INTERRUPT
    //     if(stat!=RegisterStatus::NAK) return true;//No error, just buffer full
    //     written=min<unsigned int>(size,IRQgetSizeOfInBuf());
    //     SharedMemory::instance().copyBytesTo(IRQgetInBuf(),data,written);
    //     epr.IRQsetTxDataSize(written);
    //     epr.IRQsetTxStatus(RegisterStatus::VALID);
    // } else {
    //     //BULK
    //     /*
    //      * Found the long standing issue in this driver. While writing code
    //      * which sends data continuously on EP1 BULK, the main would write two
    //      * buffers and block (if using the blocking API), and when the PC side
    //      * opens the serial port, no data would come through and the
    //      * communications stalls before it starts.
    //      * After printing the endpoint register, I noticed it switched between
    //      * these three state:
    //      * NAK, VALID and DTOG=0 SW_BUF=1, VALID and DTOG=0 SW_BUF=0.
    //      * Now, table 153 on the stm32 datasheet says that when DTOG==SW_BUF
    //      * the endpoint is in nak state.
    //      * So, filling two buffers in a row stops everything.
    //      * Solution: Force filling only one buffer.
    //      */
    //     if(IRQgetBufferCount()>=1) return true;//No err, just buffer full
    //     IRQincBufferCount();
    //     if(epr.IRQgetDtogRx()) //Actually, SW_BUF
    //     {
    //         written=min<unsigned int>(size,IRQgetSizeOfBuf1());
    //         SharedMemory::instance().copyBytesTo(IRQgetBuf1(),data,written);
    //         epr.IRQsetTxDataSize1(written);
    //     } else {
    //         written=min<unsigned int>(size,IRQgetSizeOfBuf0());
    //         SharedMemory::instance().copyBytesTo(IRQgetBuf0(),data,written);
    //         epr.IRQsetTxDataSize0(written);
    //     }
    //     epr.IRQtoggleDtogRx();
    //     /*
    //      * This is a quirk of the stm32 peripheral: when the double buffering
    //      * feature is enabled, and the endpoint is set to valid, the peripheral
    //      * assumes that both buffers are filled with valid data, but this is
    //      * not the case. When IRQwrite is first called only one buffer is
    //      * filled. If the host issued three IN transactions immediately after
    //      * the IRQwrite and before any other IRQwrite, it will get first the
    //      * buffer, and then two transactions with zero bytes. Unfortunately,
    //      * I have no idea how to fix this.
    //      */
    //     epr.IRQsetTxStatus(RegisterStatus::VALID);
    // }
    // Tracer::IRQtrace(Ut::IN_BUF_FILL,IRQgetData().epNumber,written);
    // return true;
    return false;
}

bool EndpointImpl::read(unsigned char *data, int& readBytes)
{
    // readBytes=0;
    // if(IRQgetData().enabledOut==0) return false;
    // EndpointRegister& epr=USB->endpoint[IRQgetData().epNumber];
    // RegisterStatus stat=epr.IRQgetRxStatus();
    // if(stat==RegisterStatus::STALL) return false;

    // if(IRQgetData().type==Descriptor::INTERRUPT)
    // {
    //     //INTERRUPT
    //     if(stat!=RegisterStatus::NAK) return true; //No errors, just no data
    //     readBytes=epr.IRQgetReceivedBytes();
    //     SharedMemory::instance().copyBytesFrom(data,IRQgetOutBuf(),readBytes);
    //     epr.IRQsetRxStatus(RegisterStatus::VALID);
    // } else {
    //     //BULK
    //     if(IRQgetBufferCount()==0) return true; //No errors, just no data
    //     IRQdecBufferCount();
    //     if(epr.IRQgetDtogTx()) //Actually, SW_BUF
    //     {
    //         readBytes=epr.IRQgetReceivedBytes1();
    //         SharedMemory::instance().copyBytesFrom(data,IRQgetBuf1(),readBytes);
    //     } else {
    //         readBytes=epr.IRQgetReceivedBytes0();
    //         SharedMemory::instance().copyBytesFrom(data,IRQgetBuf0(),readBytes);
    //     }
    //     epr.IRQtoggleDtogTx();
    // }
    // Tracer::IRQtrace(Ut::OUT_BUF_READ,IRQgetData().epNumber,readBytes);
    // return true;
    return false;
}

void EndpointImpl::IRQconfigureInterruptEndpoint(const unsigned char *desc)
{
    // //Get endpoint data
    // const unsigned char bEndpointAddress=desc[2];
    // const unsigned char addr=bEndpointAddress & 0xf;
    // const unsigned short wMaxPacketSize=toShort(&desc[4]);

    // const shmem_ptr ptr=SharedMemory::instance().allocate(wMaxPacketSize);
    // if(ptr==0 || wMaxPacketSize==0)
    // {
    //     Tracer::IRQtrace(Ut::OUT_OF_SHMEM);
    //     return; //Out of memory, or wMaxPacketSize==0
    // }

    // this->data.type=Descriptor::INTERRUPT;

    // USB->endpoint[addr].IRQclearEpKind();
    // USB->endpoint[addr].IRQsetType(RegisterType::INTERRUPT);

    // if(bEndpointAddress & 0x80)
    // {
    //     //IN endpoint
    //     USB->endpoint[addr].IRQsetDtogTx(false);
    //     USB->endpoint[addr].IRQsetTxBuffer(ptr,0);
    //     USB->endpoint[addr].IRQsetTxStatus(RegisterStatus::NAK);
    //     this->buf0=ptr;
    //     this->size0=wMaxPacketSize;
    //     this->data.enabledIn=1;
    // } else {
    //     //OUT endpoint
    //     USB->endpoint[addr].IRQsetDtogRx(false);
    //     USB->endpoint[addr].IRQsetRxBuffer(ptr,wMaxPacketSize);
    //     USB->endpoint[addr].IRQsetRxStatus(RegisterStatus::VALID);
    //     this->buf1=ptr;
    //     this->size1=wMaxPacketSize;
    //     this->data.enabledOut=1;
    // }
}

void EndpointImpl::IRQconfigureBulkEndpoint(const unsigned char *desc)
{
    // //Get endpoint data
    // const unsigned char bEndpointAddress=desc[2];
    // const unsigned char addr=bEndpointAddress & 0xf;
    // const unsigned short wMaxPacketSize=toShort(&desc[4]);

    // const shmem_ptr ptr0=SharedMemory::instance().allocate(wMaxPacketSize);
    // const shmem_ptr ptr1=SharedMemory::instance().allocate(wMaxPacketSize);
    // if(ptr0==0 || ptr1==0 || wMaxPacketSize==0)
    // {
    //     Tracer::IRQtrace(Ut::OUT_OF_SHMEM);
    //     return; //Out of memory, or wMaxPacketSize==0
    // }

    // this->data.type=Descriptor::BULK;
    // this->buf0=ptr0;
    // this->size0=wMaxPacketSize;
    // this->buf1=ptr1;
    // this->size1=wMaxPacketSize;

    // USB->endpoint[addr].IRQsetType(RegisterType::BULK);
    // USB->endpoint[addr].IRQsetEpKind();//Enpoint is double buffered

    // if(bEndpointAddress & 0x80)
    // {
    //     //IN endpoint
    //     USB->endpoint[addr].IRQsetDtogTx(false);
    //     USB->endpoint[addr].IRQsetDtogRx(false); //Actually, SW_BUF
    //     USB->endpoint[addr].IRQsetTxBuffer0(ptr0,0);
    //     USB->endpoint[addr].IRQsetTxBuffer1(ptr1,0);
    //     USB->endpoint[addr].IRQsetTxStatus(RegisterStatus::NAK);
    //     this->data.enabledIn=1;
    // } else {
    //     //OUT endpoint
    //     USB->endpoint[addr].IRQsetDtogRx(false);
    //     USB->endpoint[addr].IRQsetDtogTx(false); //Actually, SW_BUF
    //     USB->endpoint[addr].IRQsetRxBuffer0(ptr0,wMaxPacketSize);
    //     USB->endpoint[addr].IRQsetRxBuffer1(ptr1,wMaxPacketSize);
    //     USB->endpoint[addr].IRQsetRxStatus(RegisterStatus::VALID);
    //     this->data.enabledOut=1;
    // }
    // this->bufCount=0;
}

EndpointImpl EndpointImpl::endpoints[NUM_ENDPOINTS-1];
EndpointImpl EndpointImpl::invalidEp; //Invalid endpoint, always disabled

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY