
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

/* $Id: mem.c,v 1.113 2002/05/23 04:32:30 marka Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <isc/mem.h>
#include <isc/util.h>

void *
isc_mem_get(isc_mem_t *ctx, size_t size) {
	UNUSED(ctx);
	if(size == 0)
		size = 1;
	return (malloc(size));
}

void
isc_mem_put(isc_mem_t *ctx, void *ptr, size_t size)
{
	UNUSED(ctx);
	UNUSED(size);
	free(ptr);
}