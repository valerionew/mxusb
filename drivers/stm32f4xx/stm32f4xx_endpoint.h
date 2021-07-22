#ifndef STM32F4XX_ENDPOINT_H
#define	STM32F4XX_ENDPOINT_H

#ifndef MXUSB_LIBRARY
#error "This is header is private, it can be used only within mxusb."
#error "If your code depends on a private header, it IS broken."
#endif //MXUSB_LIBRARY

#include "usb.h"
//#include "endpoint_reg.h"
//#include "stm32_usb_regs.h"
//FIXME: include should be implementation independent
#include "drivers/stm32f4xx/stm32f4xx_peripheral.h"

#include "device_state.h"

#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

/**
 * \internal
 * Implemenation class for Endpoint facade class.
 * It contains all what should be accessible from within the mxgui library,
 * but not accessible by user code.
 */
class EndpointImpl
{
public:
    /**
     * \internal
     * Endpoint data, subject to these restrictions:
     * - Type can be Descriptor::BULK or Descriptor::INTERRUPT only
     * - If type is Descriptor::BULK only enabledIn or enabledOut can be @ 1,
     *   while if Descriptor::INTERRUPT both IN and OUT side can be enabled.
     * - epNumber!=0
     */
    struct EpData
    {
        EpData(): enabledIn(0), enabledOut(0), type(0), epNumber(1) {}

        unsigned int enabledIn:1;  ///< 1 if IN  side of endpoint enabled
        unsigned int enabledOut:1; ///< 1 if OUT side of endpoint enabled
        unsigned int type:2;       ///< Contains a Descriptor::Type enum
        unsigned int epNumber:4;   ///< Endpoint number
    };
    
    /**
     * Allows to access an endpoint.
     * \param epNum Endpoint number, must be in range 1<=epNum<maxNumEndpoints()
     * or behaviour is undefined.
     * \return the endpoint class that allows to read/write from that endpoint
     */
    static EndpointImpl *get(unsigned char epNum)
    {
        if(epNum==0 || epNum>=NUM_ENDPOINTS) return &invalidEp;
        return &endpoints[epNum-1];
    }

    /**
     * Allows to access an endpoint within IRQ or with interrupts disabled.
     * \param epNum Endpoint number, must be in range 1<=epNum<maxNumEndpoints()
     * or behaviour is undefined.
     * \return the endpoint class that allows to read/write from that endpoint
     */
    static EndpointImpl *IRQget(unsigned char epNum) { return get(epNum); }

    /**
     * Wake thread waiting on IN side of an endpoint, if there is one.
     */
    void IRQwakeWaitingThreadOnInEndpoint()
    {
        #ifdef _MIOSIX
        using namespace miosix;
        if(waitIn==0) return;
        waitIn->IRQwakeup();
        if(waitIn->getPriority()>Thread::IRQgetCurrentThread()->getPriority())
                Scheduler::IRQfindNextThread();
        waitIn=0;
        #endif //_MIOSIX
    }

    /**
     * Wake thread waiting on OUT side of an endpoint, if there is one.
     */
    void IRQwakeWaitingThreadOnOutEndpoint()
    {
        #ifdef _MIOSIX
        using namespace miosix;
        if(waitOut==0) return;
        waitOut->IRQwakeup();
        if(waitOut->getPriority()>Thread::IRQgetCurrentThread()->getPriority())
                Scheduler::IRQfindNextThread();
        waitOut=0;
        #endif //_MIOSIX
    }

    #ifdef _MIOSIX
    /**
     * Set thread waiting on IN side of an endpoint.
     */
    void IRQsetWaitingThreadOnInEndpoint(miosix::Thread *t) { waitIn=t; }

    /**
     * Set thread waiting on OUT side of an endpoint.
     */
    void IRQsetWaitingThreadOnOutEndpoint(miosix::Thread *t) { waitOut=t; }
    #endif //_MIOSIX

    /**
     * \return buf1 for double buffered BULK endpoints
     */
    //shmem_ptr IRQgetBuf1() const { return buf1; }

    /**
     * \return size of buf1 for double buffered BULK endpoints
     */
    //unsigned char IRQgetSizeOfBuf1() const { return size1; }

    /**
     * \return OUT buffer for INTERRUPT endpoints
     */
    //shmem_ptr IRQgetOutBuf() const { return buf1; }

    /**
     * \return size of OUT buffer for INTERRUPT endpoints
     */
    //unsigned char IRQgetSizeOfOutBuf() const { return size1; }

    /**
     * \return size of OUT buffer for INTERRUPT endpoints
     */
    //unsigned char getSizeOfOutBuf() const { return size1; }
    unsigned char getSizeOfOutBuf() const { return bufSize; }

    /**
     * \return buf0 for double buffered BULK endpoints
     */
    //shmem_ptr IRQgetBuf0() const { return buf0; }

    /**
     * \return size of buf0 for double buffered BULK endpoints
     */
    //unsigned char IRQgetSizeOfBuf0() const { return size0; }

    /**
     * \return IN buffer for INTERRUPT endpoints
     */
    //shmem_ptr IRQgetInBuf() const { return buf0; }

    /**
     * \return size of IN buffer for INTERRUPT endpoints
     */
    //unsigned char IRQgetSizeOfInBuf() const {  return size0; }

    /**
     * \return size of IN buffer for INTERRUPT endpoints
     */
    //unsigned char getSizeOfInBuf() const { return size0; }
    unsigned char getSizeOfInBuf() const { return bufSize; }

    /**
     * \return endpoint data
     */
    EpData IRQgetData() const { return data; }

    /**
     * \return endpoint data. Can be called with interrupts enabled.
     */
    EpData getData() const { return data; }

    /**
     * \return number of full buffers, used to understand how many buffers
     * are full/empty with double buffered BULK endpoints.
     */
    unsigned char IRQgetBufferCount() const { return bufCount; }

    /**
     * Increment number of used buffers
     */
    void IRQincBufferCount()
    {
        bufCount++;
    }

    /**
     * Decrement number of used buffers
     */
    void IRQdecBufferCount()
    {
        if(bufCount!=0) bufCount--;
    }

    /**
     * Deconfigure all endpoints
     */
    static void IRQdeconfigureAll();

    /**
     * Configure all endpoints
     * \param desc configuration descriptor
     */
    static void IRQconfigureAll(const unsigned char *desc);

    /**
     * Deconfigure this endpoint.
     * \param epNum the number of this endpoint, used to ser data.epNumber
     */
    void IRQdeconfigure(int epNum);

    /**
     * Configure this endpoint given an endpoint descriptor.
     * \param desc endpoint descriptor
     */
    void IRQconfigure(const unsigned char *desc);

    bool write(const unsigned char *data, int size, int& written);

    bool read(unsigned char *data, int& readBytes);

private:
    EndpointImpl(const EndpointImpl&);
    EndpointImpl& operator= (const EndpointImpl&);

    #ifdef _MIOSIX
    EndpointImpl(): data(), //size0(0), size1(0), buf0(0), buf1(0),
            waitIn(0), waitOut(0) {}
    #else //_MIOSIX
    EndpointImpl(): data(), size0(0), size1(0), buf0(0), buf1(0) {}
    #endif //_MIOSIX

    /**
     * Called by IRQconfigure() to set up an Interrupt endpoint
     * \param desc Must be the descriptor of an Interrupt endpoint
     */
    void IRQconfigureInterruptEndpoint(const unsigned char *desc);

    /**
     * Called by IRQconfigure() to set up an Bulk endpoint
     * \param desc Must be the descriptor of a Bulk endpoint
     */
    void IRQconfigureBulkEndpoint(const unsigned char *desc);

    // Note: size0 and size1 are unsigned char because the stm32 has a full
    // speed USB peripheral, so max buffer size for and endpoint is 64bytes

    EpData data;            ///< Endpoint data (status, type, number)
    unsigned char bufCount; ///< Buffer count, used for double buffered BULK
    //unsigned char size0;    ///< Size of buf0 (if type==BULK size0==size1)
    //unsigned char size1;    ///< Size of buf1 (if type==BULK size0==size1)
    //shmem_ptr buf0;         ///< IN  buffer for INTERRUPT, buf0 for BULK
    //shmem_ptr buf1;         ///< OUT buffer for INTERRUPT, buf1 for BULK
    unsigned char bufSize;

    #ifdef _MIOSIX
    miosix::Thread *waitIn;  ///< Thread waiting on IN side
    miosix::Thread *waitOut; ///< Thread waiting on OUT side
    #endif //_MIOSIX

    static EndpointImpl endpoints[NUM_ENDPOINTS-1];
    static EndpointImpl invalidEp; //Invalid endpoint, always disabled
};

}

#endif //_BOARD_STM32F4DISCOVERY

#endif //STM32F4XX_ENDPOINT_H