
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

/*
 * To get resource.h to include our rlimit_ident[] array of rlimit identifiers
 */

#define _RLIMIT_IDENT

#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/resourcevar.h>
#include <sys/resource.h>
#include <sys/sbuf.h>
#include <sys/types.h>
#include <sys/malloc.h>

#include <fs/pseudofs/pseudofs.h>
#include <fs/procfs/procfs.h>


int
procfs_doprocrlimit(PFS_FILL_ARGS)
{
	struct plimit *limp;
	int i;

	/*
	 * Obtain a private reference to resource limits
	 */

	PROC_LOCK(p);
	limp = lim_hold(p->p_limit);
	PROC_UNLOCK(p);

	for (i = 0; i < RLIM_NLIMITS; i++) {

		/*
		 * Add the rlimit ident
		 */

		sbuf_printf(sb, "%s ", rlimit_ident[i]);

		/*
		 * Replace RLIM_INFINITY with -1 in the string
		 */

		/*
		 * current limit
		 */

		if (limp->pl_rlimit[i].rlim_cur == RLIM_INFINITY) {
			sbuf_printf(sb, "-1 ");
		} else {
			sbuf_printf(sb, "%llu ",
			    (unsigned long long)limp->pl_rlimit[i].rlim_cur);
		}

		/*
		 * maximum limit
		 */

		if (limp->pl_rlimit[i].rlim_max == RLIM_INFINITY) {
			sbuf_printf(sb, "-1\n");
		} else {
			sbuf_printf(sb, "%llu\n",
			    (unsigned long long)limp->pl_rlimit[i].rlim_max);
		}
	}

	lim_free(limp);
	return (0);
}