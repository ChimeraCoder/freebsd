
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
#include <sys/conf.h>
#include <sys/cpuvar.h>
#include <sys/errno.h>
#include <sys/jail.h>
#include <sys/kernel.h>
#include <sys/misc.h>
#include <sys/module.h>
#include <sys/mutex.h>

cpu_core_t	cpu_core[MAXCPU];
kmutex_t	cpu_lock;
solaris_cpu_t	solaris_cpu[MAXCPU];
int		nsec_per_tick;

/*
 *  OpenSolaris subsystem initialisation.
 */
static void
opensolaris_load(void *dummy)
{
	int i;

	/*
	 * "Enable" all CPUs even though they may not exist just so
	 * that the asserts work. On FreeBSD, if a CPU exists, it is
	 * enabled.
	 */
	for (i = 0; i < MAXCPU; i++) {
		solaris_cpu[i].cpuid = i;
		solaris_cpu[i].cpu_flags &= CPU_ENABLE;
	}

	mutex_init(&cpu_lock, "OpenSolaris CPU lock", MUTEX_DEFAULT, NULL);

	nsec_per_tick = NANOSEC / hz;
}

SYSINIT(opensolaris_register, SI_SUB_OPENSOLARIS, SI_ORDER_FIRST, opensolaris_load, NULL);

static void
opensolaris_unload(void)
{
	mutex_destroy(&cpu_lock);
}

SYSUNINIT(opensolaris_unregister, SI_SUB_OPENSOLARIS, SI_ORDER_FIRST, opensolaris_unload, NULL);

static int
opensolaris_modevent(module_t mod __unused, int type, void *data __unused)
{
	int error = 0;

	switch (type) {
	case MOD_LOAD:
		utsname.nodename = prison0.pr_hostname;
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

DEV_MODULE(opensolaris, opensolaris_modevent, NULL);
MODULE_VERSION(opensolaris, 1);