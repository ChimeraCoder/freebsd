
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
/// \file       stream_flags_common.c
/// \brief      Common stuff for Stream flags coders
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "stream_flags_common.h"


const uint8_t lzma_header_magic[6] = { 0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00 };
const uint8_t lzma_footer_magic[2] = { 0x59, 0x5A };


extern LZMA_API(lzma_ret)
lzma_stream_flags_compare(
		const lzma_stream_flags *a, const lzma_stream_flags *b)
{
	// We can compare only version 0 structures.
	if (a->version != 0 || b->version != 0)
		return LZMA_OPTIONS_ERROR;

	// Check type
	if ((unsigned int)(a->check) > LZMA_CHECK_ID_MAX
			|| (unsigned int)(b->check) > LZMA_CHECK_ID_MAX)
		return LZMA_PROG_ERROR;

	if (a->check != b->check)
		return LZMA_DATA_ERROR;

	// Backward Sizes are compared only if they are known in both.
	if (a->backward_size != LZMA_VLI_UNKNOWN
			&& b->backward_size != LZMA_VLI_UNKNOWN) {
		if (!is_backward_size_valid(a) || !is_backward_size_valid(b))
			return LZMA_PROG_ERROR;

		if (a->backward_size != b->backward_size)
			return LZMA_DATA_ERROR;
	}

	return LZMA_OK;
}