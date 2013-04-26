
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
///////////////////////////////////////////////////////////////////////////////
//
/// \file       hardware_physmem.c
/// \brief      Get the total amount of physical memory (RAM)
//
//  Author:     Jonathan Nieder
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "common.h"

#include "tuklib_physmem.h"


extern LZMA_API(uint64_t)
lzma_physmem(void)
{
	// It is simpler to make lzma_physmem() a wrapper for
	// tuklib_physmem() than to hack appropriate symbol visiblity
	// support for the tuklib modules.
	return tuklib_physmem();
}