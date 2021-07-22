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

#include "stm32f4xx_memory.h"
#include "usb_util.h"

#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

//
// class SharedMemory
//

shmem_ptr SharedMemoryImpl::allocate(unsigned char ep, unsigned short size, unsigned char idx)
{
    if(size % 2 !=0) size++;
    if(currentEnd+size>END) return 0;
    unsigned short result=currentEnd;
    currentEnd+=size;
    return result;
}

void SharedMemoryImpl::reset()
{
    currentEnd=DYNAMIC_AREA;
}

void SharedMemoryImpl::copyBytesFrom(unsigned char *dest, shmem_ptr src,
        unsigned short n)
{
    //Use optimized version if dest is two words aligned
    if((reinterpret_cast<unsigned int>(dest) & 1)==0)
    {
        n=(n+1)/2;
        unsigned short *dest2=reinterpret_cast<unsigned short*>(dest);
        const unsigned int *src2=USB_RAM+(src/2);
        for(int i=0;i<n;i++) dest2[i]=src2[i];
        return;
    }
    //Use slow version if dest is not two words aligned
    const unsigned char *src2=reinterpret_cast<unsigned char*>(USB_RAM+(src/2));
    for(int i=0;i<n;i++)
    {
        *dest++=*src2++;
        //Work around the quirk that memory is 2 byte
        //of data separated by 2 bytes of gap
        if((i & 1)==1) src2+=2;
    }
}
void SharedMemoryImpl::copyBytesFrom_NEW(unsigned char *dest, unsigned char ep, unsigned short n, unsigned char idx)
{
    // TODO
}

void SharedMemoryImpl::copyBytesTo(shmem_ptr dest, const unsigned char *src,
        unsigned short n)
{
    //Use optimized version if dest is two words aligned
    if((reinterpret_cast<unsigned int>(src) & 1)==0)
    {
        n=(n+1)/2;
        const unsigned short *src2=reinterpret_cast<const unsigned short*>(src);
        unsigned int *dest2=USB_RAM+(dest/2);
        for(int i=0;i<n;i++) dest2[i]=src2[i];
        return;
    }
    //Use slow version if dest is not two words aligned
    n=(n+1) & ~1; //Rount to upper # divisible by two
    //for(int i=0;i<n;i+=2) shortAt(dest+i)=toShort(&src[i]);
}
void SharedMemoryImpl::copyBytesTo_NEW(unsigned char ep, const unsigned char *src, unsigned short n, unsigned char idx)
{
    // TODO
}

/*unsigned int& SharedMemoryImpl::shortAt(shmem_ptr ptr)
{
    return *(USB_RAM+(ptr>>1));
}

const unsigned char SharedMemoryImpl::charAt(shmem_ptr ptr)
{
    return *(reinterpret_cast<unsigned char *>(USB_RAM+(ptr>>1))+(ptr & 1));
}

const unsigned short SharedMemoryImpl::getEP0Size()
{
    return SharedMemoryImpl::EP0_SIZE;
}

const shmem_ptr SharedMemoryImpl::getEP0TxAddr()
{
    return SharedMemoryImpl::EP0TX_ADDR;
}

const shmem_ptr SharedMemoryImpl::getEP0RxAddr()
{
    return SharedMemoryImpl::EP0RX_ADDR;
}*/

shmem_ptr SharedMemoryImpl::currentEnd=DYNAMIC_AREA;

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY