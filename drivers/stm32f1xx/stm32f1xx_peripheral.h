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

#ifndef STM32F1XX_PERIPHERAL_H
#define	STM32F1XX_PERIPHERAL_H

#ifdef _MIOSIX
#include "interfaces/arch_registers.h"
#include "interfaces/delays.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f10x.h"
#endif //_MIOSIX

#include "usb_peripheral.h"
#include "drivers/stm32f1xx/stm32f1xx_memory.h"

#ifdef _BOARD_STM32F103C8_BREAKOUT

namespace mxusb {

/**
 * \internal
 * Endpoint registers are quite a bit tricky to touch, since they both have
 * "normal" bits, rc_w0 bits that can only be cleared by writing zero and
 * toggle-only bits. This makes it hard to change a bit without inadvertently
 * flipping some other.
 */
class EndpointRegister
{
public:
    /**
     * Set endpoint type
     * \param type BULK/CONTROL/ISOCHRONOUS/INTERRUPT
     */
    void IRQsetType(RegisterType type);

    /**
     * Set the way an endpoint answers IN transactions (device to host)
     * \param status DISABLED/STALL/NAK/VALID
     */
    void IRQsetTxStatus(RegisterStatus status);

    /**
     * Get the way an endpoint answers IN transactions (device to host)
     * \return status DISABLED/STALL/NAK/VALID
     */
    RegisterStatus IRQgetTxStatus() const;

    /**
     * Set the way an endpoint answers OUT transactions (host to device)
     * \param status DISABLED/STALL/NAK/VALID
     */
    void IRQsetRxStatus(RegisterStatus status);

    /**
     * Get the way an endpoint answers OUT transactions (host to device)
     * \return status DISABLED/STALL/NAK/VALID
     */
    RegisterStatus IRQgetRxStatus() const;

    /**
     * Set tx buffer for an endpoint. It is used for IN transactions
     * \param addr address of buffer, as returned by SharedMemory::allocate()
     * \param size buffer size. Size must be divisible by 2
     */
    void IRQsetTxBuffer(shmem_ptr addr, unsigned short size);

    /**
     * Set alternate tx buffer 0 for an endpoint.
     * It is used for double buffered BULK IN endpoints.
     * \param addr address of buffer, as returned by SharedMemory::allocate()
     * \param size buffer size. Size must be divisible by 2
     */
    void IRQsetTxBuffer0(shmem_ptr addr, unsigned short size);

    /**
     * Set alternate tx buffer 1 for an endpoint.
     * It is used for double buffered BULK IN endpoints.
     * \param addr address of buffer, as returned by SharedMemory::allocate()
     * \param size buffer size. Size must be divisible by 2
     */
    void IRQsetTxBuffer1(shmem_ptr addr, unsigned short size);

    /**
     * Set size of buffer to be transmitted
     * \param size buffer size
     */
    void IRQsetTxDataSize(unsigned short size);

    /**
     * Set size of alternate tx buffer 0 to be transmitted.
     * It is used for double buffered BULK IN endpoints.
     * \param size buffer size
     */
    void IRQsetTxDataSize0(unsigned short size);

    /**
     * Set size of alternate tx buffer 1 to be transmitted.
     * It is used for double buffered BULK IN endpoints.
     * \param size buffer size
     */
    void IRQsetTxDataSize1(unsigned short size);

    /**
     * Set rx buffer for an endpoint. It is used for OUT transactions
     * \param addr address of buffer, as returned by SharedMemory::allocate()
     * \param size buffer size. Due to hardware restrictions, size must respect
     * these constraints:
     * - if size is less or equal 62 bytes, it must be divisible by 2
     * - if size is more than 62 bytes, it must be a multiple of 32
     */
    void IRQsetRxBuffer(shmem_ptr addr, unsigned short size);

    /**
     * Set alternate rx buffer 0 for an endpoint.
     * It is used for double buffered BULK OUT endpoints.
     * \param addr address of buffer, as returned by SharedMemory::allocate()
     * \param size buffer size. Due to hardware restrictions, size must respect
     * these constraints:
     * - if size is less or equal 62 bytes, it must be divisible by 2
     * - if size is more than 62 bytes, it must be a multiple of 32
     */
    void IRQsetRxBuffer0(shmem_ptr addr, unsigned short size);

    /**
     * Set alternate rx buffer 1 for an endpoint.
     * It is used for double buffered BULK OUT endpoints.
     * \param addr address of buffer, as returned by SharedMemory::allocate()
     * \param size buffer size. Due to hardware restrictions, size must respect
     * these constraints:
     * - if size is less or equal 62 bytes, it must be divisible by 2
     * - if size is more than 62 bytes, it must be a multiple of 32
     */
    void IRQsetRxBuffer1(shmem_ptr addr, unsigned short size);

    /**
     * When servicing an OUT transaction, get the number of bytes that the
     * host PC sent.
     * \return the number of bytes received
     */
    unsigned short IRQgetReceivedBytes() const;

    /**
     * When servicing an OUT transaction on a double buffered BULK endpoint,
     * get the number of bytes that the host PC sent on buffer 0.
     * \return the number of bytes received
     */
    unsigned short IRQgetReceivedBytes0() const;

    /**
     * When servicing an OUT transaction on a double buffered BULK endpoint,
     * get the number of bytes that the host PC sent on buffer 1.
     * \return the number of bytes received
     */
    unsigned short IRQgetReceivedBytes1() const;

    /**
     * Clear the CTR_TX bit.
     */
    void IRQclearTxInterruptFlag();

    /**
     * Clear the CTR_RX bit.
     */
    void IRQclearRxInterruptFlag();

    /**
     * Set the EP_KIND bit.
     */
    void IRQsetEpKind();

    /**
     * Clear the EP_KIND bit.
     */
    void IRQclearEpKind();
    
    /**
     * Set the DTOG_TX bit.
     * \param value if true bit will be set to 1, else to 0
     */
    void IRQsetDtogTx(bool value);

    /**
     * Optimized version of setDtogTx that toggles the bit
     */
    void IRQtoggleDtogTx();

    /**
     * \return true if DTOG_TX is set
     */
    bool IRQgetDtogTx() const;

    /**
     * Set the DTOG_RX bit.
     * \param value if true bit will be set to 1, else to 0
     */
    void IRQsetDtogRx(bool value);

    /**
     * Optimized version of setDtogRx that toggles the bit
     */
    void IRQtoggleDtogRx();

    /**
     * \return true if DTOG_RX is set
     */
    bool IRQgetDtogRx() const;

    /**
     * Allows to assign a value to the hardware register.
     * \param value value to be stored in the EPR register.
     */
    void operator= (unsigned short value)
    {
        EPR=value;
    }

    /**
     * Allows to read the hardware register directly.
     * \return the value of the EPR register
     */
    unsigned short get() const { return EPR; }

private:
    EndpointRegister(const EndpointRegister&);
    EndpointRegister& operator= (const EndpointRegister&);

    //Endpoint register. This class is meant to be overlayed to the hardware
    //register EPnR. Therefore it can't have any other data member other than
    //this register (and no virtual functions nor constructors/destructors)
    volatile unsigned int EPR;
};



/*
 * \internal
 * Can you believe it? stm32f10x.h despite being nearly 8000 lines long doesn't
 * have the memory layout for the USB peripheral...
 */
struct USBmemoryLayout
{
    //These hardware registers are encapsulated in the Endpoint class
    EndpointRegister endpoint[NUM_ENDPOINTS];
    char reserved0[32];
    volatile unsigned short CNTR;
    short reserved1;
    volatile unsigned short ISTR;
    short reserved2;
    volatile unsigned short FNR;
    short reserved3;
    volatile unsigned short DADDR;
    short reserved4;
    volatile unsigned short BTABLE;
};

/**
 * \internal
 * Pointer that maps the USBmemoryLayout to the peripheral address in memory
 */
USBmemoryLayout* const USB=reinterpret_cast<USBmemoryLayout*>(0x40005c00);



/**
 * \internal
 * Hardware Abstraction Layer for the USB peripheral registers
 */
class USBperipheralImpl : public USBperipheral
{
public:
    void setAddress(unsigned short addr);

    void configureInterrupts();

    bool enable();

    void reset();

    void disable();

    void ep0setTxStatus(RegisterStatus status);

    void ep0setRxStatus(RegisterStatus status);

    unsigned short ep0read(unsigned char *data, int size = 0);

    void ep0reset();

    void ep0beginStatusTransaction();

    void ep0endStatusTransaction();

    bool ep0write(int size, const unsigned char *data = nullptr);
};

} //namespace mxusb

#endif //_BOARD_STM32F103C8_BREAKOUT

#endif //STM32F1XX_PERIPHERAL_H