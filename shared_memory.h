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

//#include <config/usb_config.h>

#ifndef SHARED_MEMORY_H
#define	SHARED_MEMORY_H

namespace mxusb {

///\internal
///Pointer to USB shared memory. Data is organized as 16bit integers, but
///aligned to 32bit boundaries, leaving 2 bytes gaps.
///Because of that, even if the access is performed as a pointer to int,
///the upper two bytes always read as zero
//unsigned int* const USB_RAM=reinterpret_cast<unsigned int*>(0x40006000);

/**
 * \inetrnal
 * This type represents a pointer into the shared memory area.
 * Its values can range from 0 to SharedMemory::END-1
 * The pointer is a pointer to char *, so it can address individual bytes
 */
typedef unsigned short shmem_ptr;

class SharedMemoryImpl;

/**
 * \internal
 * This class handles the memory area in stm32 microcontrollers that acts as a
 * dual-port RAM between the USB peripheral and the microcontroller.
 * This memory is used:
 * - to allocate the btable, the table with information about descriptors used
 *   by the USB peripheral
 * - to allocate the endpoint buffers
 *
 * The way this memory is handled is this:
 * - the btable is statically allocated at the bottom, with BTABLE_SIZE size
 * - then, the two buffer for endpoint 0 are statically allocated, with EP0_SIZE
 * - the rest of the memory is dynamically allocated ondemand when other
 *   endpoints are created. This memory cannot be deallocated, and is freed only
 *   when the USB device is reset or when device configuration is changed.
 */
class SharedMemory
{
public:

    static SharedMemory& instance();
    
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
     * \param src "pointer" into the shared memory, as returned by allocate()
     * Pointer must be two bytes aligned. Assuming for example that
     * result=allocate(n) was called, result, result+2, result+4 ..,
     * result+n-2 are valid pointers
     * \param n number of bytes to transfer
     */
    //void copyBytesFrom(unsigned char *dest, shmem_ptr src,unsigned short n);
    void copyBytesFrom_NEW(unsigned char *dest, unsigned char ep, unsigned short n, unsigned char idx = 0);

    /**
     * Copy data from RAM to the shared memory
     * \param dest "pointer" into the shared memory, as returned by allocate()
     * Pointer must be two bytes aligned. Assuming for example that
     * result=allocate(n) was called, result, result+2, result+4 ..,
     * result+n-2 are valid pointers.<br>
     * Note: if n is an odd number, then due to hardware limitations on
     * byte accessibility of the shared memory, one byte past the input
     * buffer size will be copied to the shared memory.
     * \param src pointer to a normal buffer already allocated in RAM
     * \param n number of bytes to transfer
     */
    //void copyBytesTo(shmem_ptr dest, const unsigned char *src,unsigned short n);
    void copyBytesTo_NEW(unsigned char ep, const unsigned char *src, unsigned short n, unsigned char idx = 0);

    /**
     * Access a short int into an endpoint.
     * \param ptr pointer into shared memory. Pointer must be two bytes aligned.
     * Assuming for example that result=allocate(n) was called,
     * result, result+2, result+4 .., result+n-2 are valid pointers for that
     * endpoint. Due to restrictions on the underlying hardware, the returned
     * reference is to an int, but only the first two bytes are accessible.
     * \return a reference to read/write into that memory location.
     */
    //unsigned int& shortAt(shmem_ptr ptr);

    /**
     * Access a byte int into an endpoint.
     * \param ptr pointer into shared memory.
     * Assuming for example that result=allocate(n) was called,
     * result, result+1, result+2 .., result+n-1 are valid pointers for that
     * endpoint.
     * \return the byte value into that memory location. Note that write access
     * is not allowed on a byte basis. To write into the shared memory, use
     * shortAt()
     */
    //const unsigned char charAt(shmem_ptr ptr);

    //const unsigned short getEP0Size();

    //const shmem_ptr getEP0TxAddr();

    //const shmem_ptr getEP0RxAddr();

private:
    SharedMemory(const SharedMemory&);
    SharedMemory& operator= (const SharedMemory&);

    SharedMemory(SharedMemoryImpl *impl);

    SharedMemoryImpl *pImpl;
};

} //namespace mxusb

#endif //SHARED_MEMORY_H
