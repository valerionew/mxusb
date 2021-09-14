#ifndef ENDPOINT_BASE_IMPL_H
#define	ENDPOINT_BASE_IMPL_H

#ifndef MXUSB_LIBRARY
#error "This is header is private, it can be used only within mxusb."
#error "If your code depends on a private header, it IS broken."
#endif //MXUSB_LIBRARY

#include "device_state.h"

namespace mxusb {

class EndpointBaseImpl
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
     * \return size of buffer for BULK/INTERRUPT endpoints
     */
    unsigned char getSizeOfBuf() const { return bufSize; }

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
     * Deconfigure this endpoint.
     * \param epNum the number of this endpoint, used to ser data.epNumber
     */
    virtual void IRQdeconfigure(int epNum);

    /**
     * Configure this endpoint given an endpoint descriptor.
     * \param desc endpoint descriptor
     */
    void IRQconfigure(const unsigned char *desc, int numEndpoints);

    virtual bool write(const unsigned char *data, int size, int& written) = 0;

    virtual bool read(unsigned char *data, int& readBytes) = 0;

private:
    EndpointBaseImpl(const EndpointBaseImpl&);
    EndpointBaseImpl& operator= (const EndpointBaseImpl&);

protected:
    #ifdef _MIOSIX
    EndpointBaseImpl(): data(), //size0(0), size1(0), buf0(0), buf1(0),
            waitIn(0), waitOut(0) {}
    #else //_MIOSIX
    EndpointBaseImpl(): data()/*, size0(0), size1(0), buf0(0), buf1(0)*/ {}
    #endif //_MIOSIX

    /**
     * Called by IRQconfigure() to set up an Interrupt endpoint
     * \param desc Must be the descriptor of an Interrupt endpoint
     */
    virtual void IRQconfigureInterruptEndpoint(const unsigned char *desc) = 0;

    /**
     * Called by IRQconfigure() to set up an Bulk endpoint
     * \param desc Must be the descriptor of a Bulk endpoint
     */
    virtual void IRQconfigureBulkEndpoint(const unsigned char *desc) = 0;

    // Note: size0 and size1 are unsigned char because the stm32 has a full
    // speed USB peripheral, so max buffer size for and endpoint is 64bytes

    EpData data;            ///< Endpoint data (status, type, number)
    unsigned char bufCount; ///< Buffer count, used for double buffered BULK
    unsigned char bufSize;

    #ifdef _MIOSIX
    miosix::Thread *waitIn;  ///< Thread waiting on IN side
    miosix::Thread *waitOut; ///< Thread waiting on OUT side
    #endif //_MIOSIX
};

} //namespace mxusb

#endif //ENDPOINT_BASE_IMPL_H