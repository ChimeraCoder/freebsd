
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
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/errno.h>
#include "opt_compat.h"
#include "opt_nfs.h"

static int
dtraceall_modevent(module_t mod __unused, int type, void *data __unused)
{
	int error = 0;

	switch (type) {
	case MOD_LOAD:
		break;

	case MOD_UNLOAD:
		break;

	case MOD_SHUTDOWN:
		break;

	default:
		error = EOPNOTSUPP;
		break;

	}

	return (error);
}

DEV_MODULE(dtraceall, dtraceall_modevent, NULL);
MODULE_VERSION(dtraceall, 1);

/* All the DTrace modules should be dependencies here: */
MODULE_DEPEND(dtraceall, cyclic, 1, 1, 1);
MODULE_DEPEND(dtraceall, opensolaris, 1, 1, 1);
MODULE_DEPEND(dtraceall, dtrace, 1, 1, 1);
MODULE_DEPEND(dtraceall, dtio, 1, 1, 1);
MODULE_DEPEND(dtraceall, dtmalloc, 1, 1, 1);
MODULE_DEPEND(dtraceall, dtnfscl, 1, 1, 1);
#if defined(NFSCLIENT)
MODULE_DEPEND(dtraceall, dtnfsclient, 1, 1, 1);
#endif
#if defined(__amd64__) || defined(__i386__) || defined(__powerpc__)
MODULE_DEPEND(dtraceall, fbt, 1, 1, 1);
#endif
#if defined(__amd64__) || defined(__i386__)
MODULE_DEPEND(dtraceall, fasttrap, 1, 1, 1);
#endif
MODULE_DEPEND(dtraceall, lockstat, 1, 1, 1);
MODULE_DEPEND(dtraceall, sdt, 1, 1, 1);
MODULE_DEPEND(dtraceall, systrace, 1, 1, 1);
#if defined(COMPAT_FREEBSD32)
MODULE_DEPEND(dtraceall, systrace_freebsd32, 1, 1, 1);
#endif
MODULE_DEPEND(dtraceall, profile, 1, 1, 1);