
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

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)kvm_getloadavg.c	8.1 (Berkeley) 6/4/93";
#endif
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <stdlib.h>
#include <limits.h>
#include <nlist.h>
#include <kvm.h>

#include "kvm_private.h"

static struct nlist nl[] = {
	{ .n_name = "_averunnable" },
#define	X_AVERUNNABLE	0
	{ .n_name = "_fscale" },
#define	X_FSCALE	1
	{ .n_name = "" },
};

/*
 * kvm_getloadavg() -- Get system load averages, from live or dead kernels.
 *
 * Put `nelem' samples into `loadavg' array.
 * Return number of samples retrieved, or -1 on error.
 */
int
kvm_getloadavg(kvm_t *kd, double loadavg[], int nelem)
{
	struct loadavg loadinfo;
	struct nlist *p;
	int fscale, i;

	if (ISALIVE(kd))
		return (getloadavg(loadavg, nelem));

	if (kvm_nlist(kd, nl) != 0) {
		for (p = nl; p->n_type != 0; ++p);
		_kvm_err(kd, kd->program,
		    "%s: no such symbol", p->n_name);
		return (-1);
	}

#define KREAD(kd, addr, obj) \
	(kvm_read(kd, addr, (char *)(obj), sizeof(*obj)) != sizeof(*obj))
	if (KREAD(kd, nl[X_AVERUNNABLE].n_value, &loadinfo)) {
		_kvm_err(kd, kd->program, "can't read averunnable");
		return (-1);
	}

	/*
	 * Old kernels have fscale separately; if not found assume
	 * running new format.
	 */
	if (!KREAD(kd, nl[X_FSCALE].n_value, &fscale))
		loadinfo.fscale = fscale;

	nelem = MIN(nelem, (int)(sizeof(loadinfo.ldavg) / sizeof(fixpt_t)));
	for (i = 0; i < nelem; i++)
		loadavg[i] = (double) loadinfo.ldavg[i] / loadinfo.fscale;
	return (nelem);
}