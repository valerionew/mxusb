#ifndef STM32F4XX_PERIPHERAL_H
#define	STM32F4XX_PERIPHERAL_H

#include "drivers/stm32f4xx/stm32f4xx_memory.h"

#ifdef _MIOSIX
#include "interfaces/arch_registers.h"
#include "interfaces/delays.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f4xx.h"
#endif //_MIOSIX

#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

/// \internal
/// Number of hardware endpoints of the stm32
const int NUM_ENDPOINTS=8;

/**
 * Note: bitmask for Descriptor::Type (bitmask used in standard USB
 * descriptors) differ from Endpoint::Type (bitmask used in stm32's EPnR
 * register bits for endpoint types)
 */
enum RegisterType
{
    BULK=0,
    CONTROL=USB_EP0R_EP_TYPE_0,
    ISOCHRONOUS=USB_EP0R_EP_TYPE_1,
    INTERRUPT=USB_EP0R_EP_TYPE_1 | USB_EP0R_EP_TYPE_0
};

enum RegisterStatus
{
    DISABLED=0,
    STALL=1<<0,
    NAK=1<<1,
    VALID=(1<<0) | (1<<1)
};

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



/**
 * \internal
 * Hardware Abstraction Layer for the USB peripheral registers
 */
class USBperipheral
{
public:
    static void setAddress(unsigned short addr);

    static void configureInterrupts();

    static bool enable();

    static void reset();

    static void disable();

    static void ep0setTxStatus(RegisterStatus status);

    static void ep0setRxStatus(RegisterStatus status);

    static unsigned short ep0getReceivedBytes();

    static void ep0reset();

    static void ep0beginStatusTransaction();

    static void ep0endStatusTransaction();

    static void ep0setTxDataSize(unsigned short size);

    static void ep0setType(RegisterType type);

    static void ep0setTxBuffer();

    static void ep0setRxBuffer();
};

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY

#endif //STM32F4XX_PERIPHERAL_H