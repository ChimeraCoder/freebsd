
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
/// \file       vli_size.c
/// \brief      Calculates the encoded size of a variable-length integer
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "common.h"


extern LZMA_API(uint32_t)
lzma_vli_size(lzma_vli vli)
{
	if (vli > LZMA_VLI_MAX)
		return 0;

	uint32_t i = 0;
	do {
		vli >>= 7;
		++i;
	} while (vli != 0);

	assert(i <= LZMA_VLI_BYTES_MAX);
	return i;
}