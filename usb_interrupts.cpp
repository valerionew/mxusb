#ifdef _MIOSIX
#include "interfaces/delays.h"
#include "kernel/kernel.h"
#include "interfaces-impl/portability_impl.h"
using namespace miosix;
#else //_MIOSIX
#include "libraries/system.h"
#endif //_MIOSIX

//
// interrupt handler
//

/**
 * \internal
 * Low priority interrupt, called for everything except double buffered
 * bulk/isochronous correct transfers.
 */
extern void USB_LP_CAN1_RX0_IRQHandler() __attribute__((naked));
void USB_LP_CAN1_RX0_IRQHandler()
{
    #ifdef _MIOSIX
    //Since a context switch can happen within this interrupt handler, it is
    //necessary to save and restore context
    saveContext();
    asm volatile("bl _ZN5mxusb15USBirqLpHandlerEv");
    restoreContext();
    #else //_MIOSIX
    asm volatile("ldr r0, =_ZN5mxusb15USBirqLpHandlerEv\n\t"
                 "bx  r0                               \n\t");
    #endif //_MIOSIX
}

/**
 * \internal
 * High priority interrupt, called for a correct transfer on double bufferes
 * bulk/isochronous endpoint.
 */
extern void USB_HP_CAN1_TX_IRQHandler() __attribute__((naked));
void USB_HP_CAN1_TX_IRQHandler()
{
    #ifdef _MIOSIX
    //Since a context switch can happen within this interrupt handler, it is
    //necessary to save and restore context
    saveContext();
    asm volatile("bl _ZN5mxusb15USBirqHpHandlerEv");
    restoreContext();
    #else //_MIOSIX
    asm volatile("ldr r0, =_ZN5mxusb15USBirqHpHandlerEv\n\t"
                 "bx  r0                               \n\t");
    #endif //_MIOSIX
}