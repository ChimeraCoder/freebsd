
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
/// \file       powerpc.c
/// \brief      Filter for PowerPC (big endian) binaries
///
//  Authors:    Igor Pavlov
//              Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "simple_private.h"


static size_t
powerpc_code(lzma_simple *simple lzma_attribute((__unused__)),
		uint32_t now_pos, bool is_encoder,
		uint8_t *buffer, size_t size)
{
	size_t i;
	for (i = 0; i + 4 <= size; i += 4) {
		// PowerPC branch 6(48) 24(Offset) 1(Abs) 1(Link)
		if ((buffer[i] >> 2) == 0x12
				&& ((buffer[i + 3] & 3) == 1)) {

			const uint32_t src = ((buffer[i + 0] & 3) << 24)
					| (buffer[i + 1] << 16)
					| (buffer[i + 2] << 8)
					| (buffer[i + 3] & (~3));

			uint32_t dest;
			if (is_encoder)
				dest = now_pos + (uint32_t)(i) + src;
			else
				dest = src - (now_pos + (uint32_t)(i));

			buffer[i + 0] = 0x48 | ((dest >> 24) &  0x03);
			buffer[i + 1] = (dest >> 16);
			buffer[i + 2] = (dest >> 8);
			buffer[i + 3] &= 0x03;
			buffer[i + 3] |= dest;
		}
	}

	return i;
}


static lzma_ret
powerpc_coder_init(lzma_next_coder *next, lzma_allocator *allocator,
		const lzma_filter_info *filters, bool is_encoder)
{
	return lzma_simple_coder_init(next, allocator, filters,
			&powerpc_code, 0, 4, 4, is_encoder);
}


extern lzma_ret
lzma_simple_powerpc_encoder_init(lzma_next_coder *next,
		lzma_allocator *allocator, const lzma_filter_info *filters)
{
	return powerpc_coder_init(next, allocator, filters, true);
}


extern lzma_ret
lzma_simple_powerpc_decoder_init(lzma_next_coder *next,
		lzma_allocator *allocator, const lzma_filter_info *filters)
{
	return powerpc_coder_init(next, allocator, filters, false);
}