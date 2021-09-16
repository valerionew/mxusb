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

#ifndef ENDPOINT_BASE_IMPL_H
#define	ENDPOINT_BASE_IMPL_H

#include "device_state.h"

namespace mxusb {

/**
 * \internal
 * Implemenation class for Endpoint facade class.
 * It contains all what should be accessible from within the mxusb library,
 * but not accessible by user code.
 */
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

    /**
     * Write data to a buffer in the shared memory and starts the IN transaction. Endpoint IN side must be enabled.
     * \param data data to write
     * \param size size of data to write
     * \param written number of bytes actually written
     * \return false in case of errors, or if the host suspended/reconfigured the device
     */
    virtual bool write(const unsigned char *data, int size, int& written) = 0;

    /**
     * Read data from an a buffer in the shared memory. Endpoint OUT side must be enabled.
     * \param data buffer where read data is stored
     * \param readBytes number of bytes actually read
     * \return false in case of errors, or if the host suspended/reconfigured the device
     */
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