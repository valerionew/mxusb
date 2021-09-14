#ifndef STM32F4XX_GPIO_H
#define	STM32F4XX_GPIO_H

#ifdef _BOARD_STM32F4DISCOVERY

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
    typedef miosix::Gpio<GPIOA_BASE,11> dm;         //USB d-
    typedef miosix::Gpio<GPIOA_BASE,12> dp;         //USB d+
    #else //_MIOSIX
    typedef Gpio<GPIOA_BASE,11> dm;         //USB d-
    typedef Gpio<GPIOA_BASE,12> dp;         //USB d+
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

        // Enable port A of GPIO. It includes leds, vbus, dp and dm.
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

        dm::mode(Mode::ALTERNATE);
        dm::speed(Speed::_100MHz);
        dm::alternateFunction(10);

        dp::mode(Mode::ALTERNATE);
        dp::speed(Speed::_100MHz);
        dp::alternateFunction(10);
    }

    /**
     * \internal
     * Enable 1.5K pullup resistor, to signal to the host that a devce has
     * been connected.
     */
    void enablePullup()
    {
        // NOTE: empty method
        // not implemented in F4 driver because the pullup resistor
        // is handled internally by the USB peripheral
    }

    /**
     * \internal
     * Disable the 1.5K pullup resistor, to signal to the host that the device
     * has been disconnected.
     */
    void disablePullup()
    {
        // NOTE: empty method
        // not implemented in F4 driver because the pullup resistor
        // is handled internally by the USB peripheral
    }
};

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY

#endif //STM32F4XX_GPIO_H