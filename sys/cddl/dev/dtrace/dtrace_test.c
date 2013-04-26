
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
#include <sys/types.h>
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/vnode.h>

/*
 * These are variables that the DTrace test suite references in the
 * Solaris kernel. We define them here so that the tests function 
 * unaltered.
 */
int	kmem_flags;

typedef struct vnode vnode_t;
vnode_t dummy;
vnode_t *rootvp = &dummy;

static int
dtrace_test_modevent(module_t mod, int type, void *data)
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

DEV_MODULE(dtrace_test, dtrace_test_modevent, NULL);
MODULE_VERSION(dtrace_test, 1);
MODULE_DEPEND(dtrace_test, dtraceall, 1, 1, 1);