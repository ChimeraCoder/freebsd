
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
 * Backend for the lock tracing (lockstat) kernel support. This is required 
 * to allow a module to load even though DTrace kernel support may not be 
 * present. 
 *
 */

#include "opt_kdtrace.h"

#ifdef KDTRACE_HOOKS

#include <sys/time.h>
#include <sys/types.h>
#include <sys/lockstat.h>

/*
 * The following must match the type definition of dtrace_probe.  It is  
 * defined this way to avoid having to rely on CDDL code.
 */
uint32_t lockstat_probemap[LS_NPROBES];
void (*lockstat_probe_func)(uint32_t, uintptr_t, uintptr_t,
    uintptr_t, uintptr_t, uintptr_t);


uint64_t 
lockstat_nsecs(void)
{
	struct bintime bt;
	uint64_t ns;

	binuptime(&bt);
	ns = bt.sec * (uint64_t)1000000000;
	ns += ((uint64_t)1000000000 * (uint32_t)(bt.frac >> 32)) >> 32;
	return (ns);
}

#endif /* KDTRACE_HOOKS */