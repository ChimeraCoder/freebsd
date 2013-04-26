
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * MD primitives supporting placement of module data 
 *
 * XXX should check load address/size against memory top.
 */
#include <stand.h>

#include "libofw.h"

#define	READIN_BUF	(4 * 1024)
#define	PAGE_SIZE	0x1000
#define	PAGE_MASK	0x0fff
#define MAPMEM_PAGE_INC 16


#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))

static int
ofw_mapmem(vm_offset_t dest, const size_t len)
{
        void    *destp, *addr;
        size_t  dlen;
        size_t  resid;
	size_t  nlen;
        static vm_offset_t last_dest = 0;
        static size_t last_len = 0;

	nlen = len;
        /*
         * Check to see if this region fits in a prior mapping.
         * Allocations are generally sequential, so only check
         * the last one.
         */
        if (dest >= last_dest &&
            (dest + len) <= (last_dest + last_len)) {
                return (0);
	}

	/*
	 * Trim area covered by existing mapping, if any
	 */
	if (dest < (last_dest + last_len) && dest >= last_dest) {
		nlen -= (last_dest + last_len) - dest;
		dest = last_dest + last_len;
	}

        destp = (void *)(dest & ~PAGE_MASK);
        resid = dest & PAGE_MASK;

	/*
	 * To avoid repeated mappings on small allocations,
	 * never map anything less than MAPMEM_PAGE_INC pages at a time
	 */
	if ((nlen + resid) < PAGE_SIZE*MAPMEM_PAGE_INC) {
		dlen = PAGE_SIZE*MAPMEM_PAGE_INC;
	} else
		dlen = roundup(nlen + resid, PAGE_SIZE);

        if (OF_call_method("claim", memory, 3, 1, destp, dlen, 0, &addr)
            == -1) {
                printf("ofw_mapmem: physical claim failed\n");
                return (ENOMEM);
        }

	/*
	 * We only do virtual memory management when real_mode is false.
	 */
	if (real_mode == 0) {
		if (OF_call_method("claim", mmu, 3, 1, destp, dlen, 0, &addr)
		    == -1) {
			printf("ofw_mapmem: virtual claim failed\n");
			return (ENOMEM);
		}

		if (OF_call_method("map", mmu, 4, 0, destp, destp, dlen, 0)
		    == -1) {
			printf("ofw_mapmem: map failed\n");
			return (ENOMEM);
		}
	}
        last_dest = (vm_offset_t) destp;
        last_len  = dlen;

        return (0);
}

ssize_t
ofw_copyin(const void *src, vm_offset_t dest, const size_t len)
{
        if (ofw_mapmem(dest, len)) {
                printf("ofw_copyin: map error\n");
                return (0);
        }

        bcopy(src, (void *)dest, len);
        return(len);
}

ssize_t
ofw_copyout(const vm_offset_t src, void *dest, const size_t len)
{
	bcopy((void *)src, dest, len);
	return(len);
}

ssize_t
ofw_readin(const int fd, vm_offset_t dest, const size_t len)
{
	void		*buf;
	size_t		resid, chunk, get;
	ssize_t		got;
	vm_offset_t	p;

	p = dest;

	chunk = min(READIN_BUF, len);
	buf = malloc(chunk);
	if (buf == NULL) {
		printf("ofw_readin: buf malloc failed\n");
		return(0);
	}

        if (ofw_mapmem(dest, len)) {
                printf("ofw_readin: map error\n");
                free(buf);
                return (0);
        }

	for (resid = len; resid > 0; resid -= got, p += got) {
		get = min(chunk, resid);
		got = read(fd, buf, get);

		if (got <= 0) {
			if (got < 0)
				printf("ofw_readin: read failed\n");
			break;
		}

		bcopy(buf, (void *)p, got);
	}

	free(buf);
	return(len - resid);
}