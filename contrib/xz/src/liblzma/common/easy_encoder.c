
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
/// \file       easy_encoder.c
/// \brief      Easy .xz Stream encoder initialization
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "easy_preset.h"
#include "stream_encoder.h"


extern LZMA_API(lzma_ret)
lzma_easy_encoder(lzma_stream *strm, uint32_t preset, lzma_check check)
{
	lzma_options_easy opt_easy;
	if (lzma_easy_preset(&opt_easy, preset))
		return LZMA_OPTIONS_ERROR;

	return lzma_stream_encoder(strm, opt_easy.filters, check);
}