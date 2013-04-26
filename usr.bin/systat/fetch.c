
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

#ifdef lint
static const char sccsid[] = "@(#)fetch.c	8.1 (Berkeley) 6/6/93";
#endif

#include <sys/types.h>
#include <sys/sysctl.h>

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "systat.h"
#include "extern.h"

int
kvm_ckread(void *a, void *b, int l)
{
	if (kvm_read(kd, (u_long)a, b, l) != l) {
		if (verbose)
			error("error reading kmem at %p", a);
		return (0);
	}
	else
		return (1);
}

void getsysctl(const char *name, void *ptr, size_t len)
{
	size_t nlen = len;
	if (sysctlbyname(name, ptr, &nlen, NULL, 0) != 0) {
		error("sysctl(%s...) failed: %s", name,
		    strerror(errno));
	}
	if (nlen != len) {
		error("sysctl(%s...) expected %lu, got %lu", name,
		    (unsigned long)len, (unsigned long)nlen);
    }
}

/*
 * Read sysctl data with variable size. Try some times (with increasing
 * buffers), fail if still too small.
 * This is needed sysctls with possibly raplidly increasing data sizes,
 * but imposes little overhead in the case of constant sizes.
 * Returns NULL on error, or a pointer to freshly malloc()'ed memory that holds
 * the requested data.
 * If szp is not NULL, the size of the returned data will be written into *szp.
 */

/* Some defines: Number of tries. */
#define SD_NTRIES  10
/* Percent of over-allocation (initial) */
#define SD_MARGIN  10
/*
 * Factor for over-allocation in percent (the margin is increased by this on
 * any failed try).
 */
#define SD_FACTOR  50
/* Maximum supported MIB depth */
#define SD_MAXMIB  16

char *
sysctl_dynread(const char *n, size_t *szp)
{
	char   *rv = NULL;
	int    mib[SD_MAXMIB];
	size_t mibsz = SD_MAXMIB;
	size_t mrg = SD_MARGIN;
	size_t sz;
	int i;

	/* cache the MIB */
	if (sysctlnametomib(n, mib, &mibsz) == -1) {
		if (errno == ENOMEM) {
			error("XXX: SD_MAXMIB too small, please bump!");
		}
		return NULL;
	}
	for (i = 0; i < SD_NTRIES; i++) {
		/* get needed buffer size */
		if (sysctl(mib, mibsz, NULL, &sz, NULL, 0) == -1)
			break;
		sz += sz * mrg / 100;
		if ((rv = (char *)malloc(sz)) == NULL) {
			error("Out of memory!");
			return NULL;
		}
		if (sysctl(mib, mibsz, rv, &sz, NULL, 0) == -1) {
			free(rv);
			rv = NULL;
			if (errno == ENOMEM) {
				mrg += mrg * SD_FACTOR / 100;
			} else
				break;
		} else {
			/* success */
			if (szp != NULL)
				*szp = sz;
			break;
		}
	}

	return rv;
}