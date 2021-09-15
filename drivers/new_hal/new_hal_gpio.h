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