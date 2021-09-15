#ifndef NEW_HAL_GPIO_H
#define	NEW_HAL_GPIO_H

#ifdef MIOSIX_DEFINE_BOARD_NAME

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

        // TODO: add here configuration of USB GPIOs
    }

    /**
     * \internal
     * Enable 1.5K pullup resistor, to signal to the host that a devce has
     * been connected.
     */
    void enablePullup()
    {
        // TODO: add here enabling of pullup resistor
    }

    /**
     * \internal
     * Disable the 1.5K pullup resistor, to signal to the host that the device
     * has been disconnected.
     */
    void disablePullup()
    {
        // TODO: add here disabling of pullup resistor
    }
};

} //namespace mxusb

#endif //MIOSIX_DEFINE_BOARD_NAME

#endif //NEW_HAL_GPIO_H