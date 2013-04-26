
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

#include <sys/malloc.h>
#include <sys/kernel.h>
#include "xz_malloc.h"

/* Wraper for XZ decompressor memmory pool */

static MALLOC_DEFINE(XZ_DEC, "XZ_DEC", "XZ decompressor data");

void *
xz_malloc(unsigned long size)
{
	void *addr;

	addr = malloc(size, XZ_DEC, M_NOWAIT);
	return (addr);
}

void
xz_free(void *addr)
{

	free(addr, XZ_DEC);
}