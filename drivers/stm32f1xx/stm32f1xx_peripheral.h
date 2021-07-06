#ifndef STM32F1XX_PERIPHERAL_H
#define	STM32F1XX_PERIPHERAL_H

#include "drivers/stm32f1xx/stm32f1xx_memory.h"

#ifdef _MIOSIX
#include "interfaces/arch_registers.h"
#include "interfaces/delays.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f10x.h"
#endif //_MIOSIX

#ifdef _BOARD_STM32F103C8_BREAKOUT

namespace mxusb {

/// \internal
/// Number of hardware endpoints of the stm32
const int NUM_ENDPOINTS=8;

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
     * Note: bitmask for Descriptor::Type (bitmask used in standard USB
     * descriptors) differ from Endpoint::Type (bitmask used in stm32's EPnR
     * register bits for endpoint types)
     */
    enum Type
    {
        BULK=0,
        CONTROL=USB_EP0R_EP_TYPE_0,
        ISOCHRONOUS=USB_EP0R_EP_TYPE_1,
        INTERRUPT=USB_EP0R_EP_TYPE_1 | USB_EP0R_EP_TYPE_0
    };

    enum Status
    {
        DISABLED=0,
        STALL=1<<0,
        NAK=1<<1,
        VALID=(1<<0) | (1<<1)
    };

    /**
     * Set endpoint type
     * \param type BULK/CONTROL/ISOCHRONOUS/INTERRUPT
     */
    void IRQsetType(Type type);

    /**
     * Set the way an endpoint answers IN transactions (device to host)
     * \param status DISABLED/STALL/NAK/VALID
     */
    void IRQsetTxStatus(Status status);

    /**
     * Get the way an endpoint answers IN transactions (device to host)
     * \return status DISABLED/STALL/NAK/VALID
     */
    Status IRQgetTxStatus() const;

    /**
     * Set the way an endpoint answers OUT transactions (host to device)
     * \param status DISABLED/STALL/NAK/VALID
     */
    void IRQsetRxStatus(Status status);

    /**
     * Get the way an endpoint answers OUT transactions (host to device)
     * \return status DISABLED/STALL/NAK/VALID
     */
    Status IRQgetRxStatus() const;

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
class USBperipheral
{
public:
    static EndpointRegister& getEndpoint(unsigned char epNum)
    {
        // TODO: check epNum bounds
        return USB->endpoint[epNum];
    }

    static void setEndpoint(int epNum)
    {
        // TODO: check epNum bounds
        USB->endpoint[epNum] = epNum;
    }

    static void setAddress(unsigned short addr)
    {
        USB->DADDR = addr;
    }

    static void configureInterrupts()
    {
        //Configure interrupts
        NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
        NVIC_SetPriority(USB_HP_CAN1_TX_IRQn,3);//Higher priority (Max=0, min=15)
        NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
        NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn,4);//High priority (Max=0, min=15)
    }

    static bool enable()
    {
        //Enable clock to USB peripheral
        #if __CM3_CMSIS_VERSION >= 0x010030 //CMSIS 1.3 changed variable names
        const int clock=SystemCoreClock;
        #else //__CM3_CMSIS_VERSION
        const int clock=SystemFrequency;
        #endif //__CM3_CMSIS_VERSION
        if(clock==72000000)
            RCC->CFGR &= ~RCC_CFGR_USBPRE; //Prescaler=1.5 (72MHz/1.5=48MHz)
        else if(clock==48000000)
            RCC->CFGR |= RCC_CFGR_USBPRE;  //Prescaler=1   (48MHz)
        else {
            //USB can't work with other clock frequencies
            #ifndef _MIOSIX
            __enable_irq();
            #endif //_MIOSIX
            return false;
        }
        RCC->APB1ENR |= RCC_APB1ENR_USBEN;
        return true;
    }

    static void reset()
    {
        USB->CNTR=USB_CNTR_FRES; //Clear PDWN, leave FRES asserted
        delayUs(1);  //Wait till USB analog circuitry stabilizes
        USB->CNTR=0; //Clear FRES too, USB peripheral active
        USB->ISTR=0; //Clear interrupt flags
        
        //First thing the host does is reset, so wait for that interrupt only
        USB->CNTR=USB_CNTR_RESETM;
    }

    static void disable()
    {
        USB->DADDR=0;  //Clear EF bit
        USB->CNTR=USB_CNTR_PDWN | USB_CNTR_FRES;
        USB->ISTR=0; //Clear interrupt flags
        RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
    }
};

} //namespace mxusb

#endif //_BOARD_STM32F103C8_BREAKOUT

#endif //STM32F1XX_PERIPHERAL_H