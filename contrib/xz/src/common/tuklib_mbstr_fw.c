
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
/// \file       tuklib_mstr_fw.c
/// \brief      Get the field width for printf() e.g. to align table columns
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "tuklib_mbstr.h"


extern int
tuklib_mbstr_fw(const char *str, int columns_min)
{
	size_t len;
	const size_t width = tuklib_mbstr_width(str, &len);
	if (width == (size_t)-1)
		return -1;

	if (width > (size_t)columns_min)
		return 0;

	if (width < (size_t)columns_min)
		len += (size_t)columns_min - width;

	return len;
}