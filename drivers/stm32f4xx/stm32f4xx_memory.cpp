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

#ifdef _MIOSIX
#include "kernel/kernel.h"
#include "interfaces/arch_registers.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f4xx.h"
#endif //_MIOSIX

#ifdef _BOARD_STM32F4DISCOVERY

namespace mxusb {

//
// class SharedMemory
//

#define MAX_PACKET_SIZE 128
#define MAX_RAM_SIZE    320 // in words
#define RX_FIFO_SIZE    (10 + ((MAX_PACKET_SIZE / 4) + 1) + ((NUM_ENDPOINTS + 1) * 2) + 1)

inline static uint32_t* FIFO(unsigned char ep) {
    return (uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + (ep << 12));
}

shmem_ptr SharedMemoryImpl::allocate(unsigned char ep, unsigned short size, unsigned char idx)
{
    //note: idx is ignored because the USB peripheral does not support double buffering

    //Calculate TX fifo size in words
    size = (size + 0x03) >> 2;
    //Check if is at least 16 words
    if (size < 0x10) size = 0x10;

    //Check if RAM is full
    if ((currentEnd + size) > MAX_RAM_SIZE) return 0;

    unsigned short addr = currentEnd;
    currentEnd += size;

    USB_OTG_FS->DIEPTXF[ep - 1] = addr | (size << 16);

    return addr;
}

void SharedMemoryImpl::reset()
{
    //Set max RX FIFO size
    USB_OTG_FS->GRXFSIZ = RX_FIFO_SIZE;
    //Set EP0 TX FIFO size
    USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = RX_FIFO_SIZE | (0x10 << 16);

    currentEnd = RX_FIFO_SIZE + 0x10;
}

void SharedMemoryImpl::copyBytesFrom_NEW(unsigned char *dest, unsigned char ep, unsigned short n, unsigned char idx)
{
    //note: idx is ignored because the USB peripheral does not support double buffering

    volatile uint32_t *fifo = FIFO(0);

    if (!(USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_RXFLVL)) return;
    if ((USB_OTG_FS->GRXSTSR & USB_OTG_GRXSTSP_EPNUM) != (ep & 0xF)) return;

    //Pop data from fifo
    uint32_t len = (USB_OTG_FS->GRXSTSP & USB_OTG_GRXSTSP_BCNT) >> 4;
    uint32_t tmp;
    // FIXME: improve logic and transform byte count to word count
    for (int idx = 0; idx < len; idx++) {
        if ((idx & 0x03) == 0x00) {
            tmp = *fifo;
        }
        if (idx < n) {
            ((uint8_t*)dest)[idx] = tmp & 0xFF;
            tmp >>= 8;
        }
    }
}

void SharedMemoryImpl::copyBytesTo_NEW(unsigned char ep, const unsigned char *src, unsigned short n, unsigned char idx)
{
    //note: idx is ignored because the USB peripheral does not support double buffering

    volatile uint32_t *fifo = FIFO(ep);
    
    //Push data to fifo
    uint32_t len = (n + 0x03) >> 2;
    uint32_t tmp = 0;
    // FIXME: improve logic and transform byte count to word count
    for (int idx = 0; idx < n; idx++) {
        tmp |= (uint32_t)((uint8_t*)src)[idx] << ((idx & 0x03) << 3);
        if ((idx & 0x03) == 0x03 || (idx+1) == n) {
            *fifo = tmp;
            tmp = 0;
        }
    }
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

shmem_ptr SharedMemoryImpl::currentEnd = 0;

} //namespace mxusb

#endif //_BOARD_STM32F4DISCOVERY