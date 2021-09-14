#ifndef STM32F1XX_GPIO_H
#define	STM32F1XX_GPIO_H

#ifdef _BOARD_STM32F103C8_BREAKOUT

#ifdef _MIOSIX
#include "interfaces/gpio.h"
#else //_MIOSIX
#include "libraries/gpio.h"
#endif //_MIOSIX

#include "usb_gpio.h"

namespace mxusb {

/**
 * This class provides support for configuring USB GPIO pins, in particular
 * the pin connected to a 1.5K pullup to signal to the host that a device is
 * present. It might need to be adapted to match the circuit schematic of
 * the board used, since the GPIO that can be used for this purpose is not
 * unique.
 */
class USBgpioImpl : public USBgpio
{
private:
    /*
     * Note: change these to reflect the hardware set up.
     */
    #ifdef _MIOSIX
    typedef miosix::Gpio<GPIOA_BASE,11> dp;         //USB d+
    typedef miosix::Gpio<GPIOA_BASE,12> dm;         //USB d-
    typedef miosix::Gpio<GPIOB_BASE,14> disconnect; //USB disconnect
    #else //_MIOSIX
    typedef Gpio<GPIOA_BASE,11> dp;         //USB d+
    typedef Gpio<GPIOA_BASE,12> dm;         //USB d-
    typedef Gpio<GPIOB_BASE,14> disconnect; //USB disconnect
    #endif //_MIOSIX

public:
    /**
     * \internal
     * Initializes USB related GPIOs
     */
    void init()
    {
        #ifdef _MIOSIX
        using namespace miosix;
        #endif //_MIOSIX
        //Enable portB (USB disconnect) and afio
        RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

        disconnect::mode(Mode::OPEN_DRAIN);
        disconnect::high();
    }

    /**
     * \internal
     * Enable 1.5K pullup resistor, to signal to the host that a devce has
     * been connected.
     */
    void enablePullup()
    {
        disconnect::low();
    }

    /**
     * \internal
     * Disable the 1.5K pullup resistor, to signal to the host that the device
     * has been disconnected.
     */
    void disablePullup()
    {
        disconnect::high();
    }
};

} //namespace mxusb

#endif //_BOARD_STM32F103C8_BREAKOUT

#endif //STM32F1XX_GPIO_H