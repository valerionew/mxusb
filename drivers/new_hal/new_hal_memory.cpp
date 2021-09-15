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

#include "new_hal_memory.h"
#include "usb_util.h"

#ifdef _MIOSIX
#include "kernel/kernel.h"
#include "interfaces/arch_registers.h"
using namespace miosix;
#else //_MIOSIX
#include "stm32f4xx.h"
#endif //_MIOSIX

#ifdef MIOSIX_DEFINE_BOARD_NAME

namespace mxusb {

//
// class SharedMemory
//

shmem_ptr SharedMemoryImpl::allocate(unsigned char ep, unsigned short size, unsigned char idx)
{
    // TODO: add here allocation of buffer in shared RAM

    unsigned short addr = currentEnd;
    currentEnd += size;

    return addr;
}

void SharedMemoryImpl::reset()
{
    // TODO: add here reset of buffers in shared RAM

    currentEnd = 0;
}

void SharedMemoryImpl::copyBytesFrom(unsigned char *dest, unsigned char ep, unsigned short n, unsigned char idx)
{
    // TODO: add here read of buffer from shared RAM
}

void SharedMemoryImpl::copyBytesTo(unsigned char ep, const unsigned char *src, unsigned short n, unsigned char idx)
{
    // TODO: add here write of buffer in shared RAM
}

shmem_ptr SharedMemoryImpl::currentEnd = 0;

} //namespace mxusb

#endif //MIOSIX_DEFINE_BOARD_NAME