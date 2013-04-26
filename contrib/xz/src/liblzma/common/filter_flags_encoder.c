
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
/// \file       filter_flags_encoder.c
/// \brief      Decodes a Filter Flags field
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "filter_encoder.h"


extern LZMA_API(lzma_ret)
lzma_filter_flags_size(uint32_t *size, const lzma_filter *filter)
{
	if (filter->id >= LZMA_FILTER_RESERVED_START)
		return LZMA_PROG_ERROR;

	return_if_error(lzma_properties_size(size, filter));

	*size += lzma_vli_size(filter->id) + lzma_vli_size(*size);

	return LZMA_OK;
}


extern LZMA_API(lzma_ret)
lzma_filter_flags_encode(const lzma_filter *filter,
		uint8_t *out, size_t *out_pos, size_t out_size)
{
	// Filter ID
	if (filter->id >= LZMA_FILTER_RESERVED_START)
		return LZMA_PROG_ERROR;

	return_if_error(lzma_vli_encode(filter->id, NULL,
			out, out_pos, out_size));

	// Size of Properties
	uint32_t props_size;
	return_if_error(lzma_properties_size(&props_size, filter));
	return_if_error(lzma_vli_encode(props_size, NULL,
			out, out_pos, out_size));

	// Filter Properties
	if (out_size - *out_pos < props_size)
		return LZMA_PROG_ERROR;

	return_if_error(lzma_properties_encode(filter, out + *out_pos));

	*out_pos += props_size;

	return LZMA_OK;
}