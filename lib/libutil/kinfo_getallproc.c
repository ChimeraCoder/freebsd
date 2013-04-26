
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

#include <sys/param.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <string.h>

#include "libutil.h"


/*
 * Sort processes first by pid and then tid.
 */
static int
kinfo_proc_compare(const void *a, const void *b)
{
	int i;

	i = ((const struct kinfo_proc *)a)->ki_pid -
	    ((const struct kinfo_proc *)b)->ki_pid;
	if (i != 0)
		return (i);
	i = ((const struct kinfo_proc *)a)->ki_tid -
	    ((const struct kinfo_proc *)b)->ki_tid;
	return (i);
}

static void
kinfo_proc_sort(struct kinfo_proc *kipp, int count)
{

	qsort(kipp, count, sizeof(*kipp), kinfo_proc_compare);
}

struct kinfo_proc *
kinfo_getallproc(int *cntp)
{
	struct kinfo_proc *kipp;
	size_t len;
	int mib[3];

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PROC;

	len = 0;
	if (sysctl(mib, 3, NULL, &len, NULL, 0) < 0)
		return (NULL);

	kipp = malloc(len);
	if (kipp == NULL)
		return (NULL);

	if (sysctl(mib, 3, kipp, &len, NULL, 0) < 0)
		goto bad;
	if (len % sizeof(*kipp) != 0)
		goto bad;
	if (kipp->ki_structsize != sizeof(*kipp))
		goto bad;
	*cntp = len / sizeof(*kipp);
	kinfo_proc_sort(kipp, len / sizeof(*kipp));
	return (kipp);
bad:
	*cntp = 0;
	free(kipp);
	return (NULL);
}