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

#include <config/usb_config.h>
#include "new_hal_config.h"
#include "shared_memory.h"

#ifndef NEW_HAL_MEMORY_H
#define	NEW_HAL_MEMORY_H

#ifdef MIOSIX_DEFINE_BOARD_NAME

namespace mxusb {

/**
 * \internal
 * This class handles the memory area in stm32 microcontrollers that acts as a
 * dual-port RAM between the USB peripheral and the microcontroller.
 */
class SharedMemoryImpl : public SharedMemory
{
public:
    SharedMemoryImpl() {}

    /**
     * Allocate space for an endpoint
     * \param size memory size required (in bytes)
     * \return the beginning of the allocated memory area. Returned pointer is
     * always two bytes aligned (last bit is zero). If not enough memory is
     * available, zero is returned
     */
    shmem_ptr allocate(unsigned char ep, unsigned short size, unsigned char idx = 0);

    /**
     * This invalidates all memory allocated with allocate
     */
    void reset();

    /**
     * Copy data from the shared memory to RAM
     * \param dest pointer to a normal buffer already allocated in RAM
     * If n is an odd number, dest should be larger than n by at least one.
     * This is because for speed reasons this function might write one byte
     * more when n is odd.
     * \param ep is the index of the endpoint from which copying data.
     * \param n number of bytes to transfer
     * \param idx is the index of the buffer and it is used only when the peripheral supports double buffer
     */
    void copyBytesFrom(unsigned char *dest, unsigned char ep, unsigned short n, unsigned char idx = 0);

    /**
     * Copy data from RAM to the shared memory
     * \param ep is the index of the endpoint to which copying data.
     * \param src pointer to a normal buffer already allocated in RAM
     * \param n number of bytes to transfer
     * \param idx is the index of the buffer and it is used only when the peripheral supports double buffer
     */
    void copyBytesTo(unsigned char ep, const unsigned char *src, unsigned short n, unsigned char idx = 0);

private:
    SharedMemoryImpl(const SharedMemoryImpl&);
    SharedMemoryImpl& operator= (const SharedMemoryImpl&);

    static shmem_ptr currentEnd;/// Pointer to the first free byte
};

} //namespace mxusb

#endif //MIOSIX_DEFINE_BOARD_NAME

#endif //NEW_HAL_MEMORY_H
