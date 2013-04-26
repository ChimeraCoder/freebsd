
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
/// \file       crc64_tablegen.c
/// \brief      Generate crc64_table_le.h and crc64_table_be.h
///
/// Compiling: gcc -std=c99 -o crc64_tablegen crc64_tablegen.c
/// Add -DWORDS_BIGENDIAN to generate big endian table.
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "../../common/tuklib_integer.h"


static uint64_t crc64_table[4][256];


extern void
init_crc64_table(void)
{
	static const uint64_t poly64 = UINT64_C(0xC96C5795D7870F42);

	for (size_t s = 0; s < 4; ++s) {
		for (size_t b = 0; b < 256; ++b) {
			uint64_t r = s == 0 ? b : crc64_table[s - 1][b];

			for (size_t i = 0; i < 8; ++i) {
				if (r & 1)
					r = (r >> 1) ^ poly64;
				else
					r >>= 1;
			}

			crc64_table[s][b] = r;
		}
	}

#ifdef WORDS_BIGENDIAN
	for (size_t s = 0; s < 4; ++s)
		for (size_t b = 0; b < 256; ++b)
			crc64_table[s][b] = bswap64(crc64_table[s][b]);
#endif

	return;
}


static void
print_crc64_table(void)
{
	printf("/* This file has been automatically generated by "
			"crc64_tablegen.c. */\n\n"
			"const uint64_t lzma_crc64_table[4][256] = {\n\t{");

	for (size_t s = 0; s < 4; ++s) {
		for (size_t b = 0; b < 256; ++b) {
			if ((b % 2) == 0)
				printf("\n\t\t");

			printf("UINT64_C(0x%016" PRIX64 ")",
					crc64_table[s][b]);

			if (b != 255)
				printf(",%s", (b+1) % 2 == 0 ? "" : " ");
		}

		if (s == 3)
			printf("\n\t}\n};\n");
		else
			printf("\n\t}, {");
	}

	return;
}


int
main(void)
{
	init_crc64_table();
	print_crc64_table();
	return 0;
}