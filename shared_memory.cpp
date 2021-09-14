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

#include "shared_memory.h"
#include "usb_util.h"

#include "drivers/stm32f1xx/stm32f1xx_memory.h"
#include "drivers/stm32f4xx/stm32f4xx_memory.h"

namespace mxusb {

//
// class SharedMemory
//

SharedMemory& SharedMemory::instance()
{
    static SharedMemoryImpl implementation;
    static SharedMemory singleton(&implementation);
    return singleton;
}

shmem_ptr SharedMemory::allocate(unsigned char ep, unsigned short size, unsigned char idx)
{
    return pImpl->allocate(ep, size, idx);
}

void SharedMemory::reset()
{
    pImpl->reset();
}

void SharedMemory::copyBytesFrom(unsigned char *dest, unsigned char ep, unsigned short n, unsigned char idx)
{
    pImpl->copyBytesFrom(dest, ep, n, idx);
}

void SharedMemory::copyBytesTo(unsigned char ep, const unsigned char *src, unsigned short n, unsigned char idx)
{
    pImpl->copyBytesTo(ep, src, n, idx);
}

SharedMemory::SharedMemory(SharedMemoryImpl *impl) : pImpl(impl) {}

} //namespace mxusb
