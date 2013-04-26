
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

struct kinfo_proc *
kinfo_getproc(pid_t pid)
{
	struct kinfo_proc *kipp;
	int mib[4];
	size_t len;

	len = 0;
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = pid;
	if (sysctl(mib, 4, NULL, &len, NULL, 0) < 0)
		return (NULL);

	kipp = malloc(len);
	if (kipp == NULL)
		return (NULL);

	if (sysctl(mib, 4, kipp, &len, NULL, 0) < 0)
		goto bad;
	if (len != sizeof(*kipp))
		goto bad;
	if (kipp->ki_structsize != sizeof(*kipp))
		goto bad;
	if (kipp->ki_pid != pid)
		goto bad;
	return (kipp);
bad:
	free(kipp);
	return (NULL);
}