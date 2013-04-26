
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
/// \file       crc32_small.c
/// \brief      CRC32 calculation (size-optimized)
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "check.h"


uint32_t lzma_crc32_table[1][256];


static void
crc32_init(void)
{
	static const uint32_t poly32 = UINT32_C(0xEDB88320);

	for (size_t b = 0; b < 256; ++b) {
		uint32_t r = b;
		for (size_t i = 0; i < 8; ++i) {
			if (r & 1)
				r = (r >> 1) ^ poly32;
			else
				r >>= 1;
		}

		lzma_crc32_table[0][b] = r;
	}

	return;
}


extern void
lzma_crc32_init(void)
{
	mythread_once(crc32_init);
	return;
}


extern LZMA_API(uint32_t)
lzma_crc32(const uint8_t *buf, size_t size, uint32_t crc)
{
	lzma_crc32_init();

	crc = ~crc;

	while (size != 0) {
		crc = lzma_crc32_table[0][*buf++ ^ (crc & 0xFF)] ^ (crc >> 8);
		--size;
	}

	return ~crc;
}