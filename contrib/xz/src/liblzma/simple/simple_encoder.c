
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
/// \file       simple_encoder.c
/// \brief      Properties encoder for simple filters
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "simple_encoder.h"


extern lzma_ret
lzma_simple_props_size(uint32_t *size, const void *options)
{
	const lzma_options_bcj *const opt = options;
	*size = (opt == NULL || opt->start_offset == 0) ? 0 : 4;
	return LZMA_OK;
}


extern lzma_ret
lzma_simple_props_encode(const void *options, uint8_t *out)
{
	const lzma_options_bcj *const opt = options;

	// The default start offset is zero, so we don't need to store any
	// options unless the start offset is non-zero.
	if (opt == NULL || opt->start_offset == 0)
		return LZMA_OK;

	unaligned_write32le(out, opt->start_offset);

	return LZMA_OK;
}