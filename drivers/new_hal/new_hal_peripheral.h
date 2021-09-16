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

#ifndef NEW_HAL_PERIPHERAL_H
#define	NEW_HAL_PERIPHERAL_H

#ifdef _MIOSIX
#include "interfaces/arch_registers.h"
#include "interfaces/delays.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f4xx.h"
#endif //_MIOSIX

#include "usb_peripheral.h"
#include "drivers/new_hal/new_hal_memory.h"

#ifdef MIOSIX_DEFINE_BOARD_NAME

namespace mxusb {

/**
 * \internal
 * Hardware Abstraction Layer for the USB peripheral registers
 */
class USBperipheralImpl : public USBperipheral
{
public:

    /**
     * \brief It set the address of the peripheral.
     * \param addr it's the address to set.
     */
    void setAddress(unsigned short addr);

    /**
     * \brief It configures the interrupts of the peripheral.
     */
    void configureInterrupts();

    /**
     * \brief It enables the USB peripheral.
     */
    bool enable();

    /**
     * \brief It resets the peripheral and clears all the interrupts
     */
    void reset();

    /**
     * \brief It resets the peripheral and power down the device
     */
    void disable();

    /**
     * \brief It sets the status of the transmit endpoint 0.
     * \param status is the status to set.
     */
    void ep0setTxStatus(RegisterStatus status);

    /**
     * \brief It sets the status of the receive endpoint 0.
     * \param status is the status to set.
     */
    void ep0setRxStatus(RegisterStatus status);

    /**
     * \brief It reads from endpoint 0.
     * \param size it's the size to read.
     * \param data is the pointer where to store the data read.
     */
    unsigned short ep0read(unsigned char *data, int size = 0);

    /**
     * \brief It resets the endpoint 0.
     */
    void ep0reset();

    /**
     * \brief Begins a status transaction on the ep0 (currently implemented only in the F1 driver).
     */
    void ep0beginStatusTransaction();

    /**
     * \brief Ends a status transaction on the ep0 (currently implemented only in the F1 driver).
     */
    void ep0endStatusTransaction();

    /**
     * \brief This method writes to the endpoint0.
     * \param size it's the size to write.
     * \param data is the pointer to the data to write.
     */
    bool ep0write(int size, const unsigned char *data = nullptr);
};

} //namespace mxusb

#endif //MIOSIX_DEFINE_BOARD_NAME

#endif //NEW_HAL_PERIPHERAL_H