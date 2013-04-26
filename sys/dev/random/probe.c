
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

#if defined(__amd64__) || (defined(__i386__) && !defined(PC98))
#include "opt_cpu.h"
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/random.h>
#include <sys/selinfo.h>
#include <sys/sysctl.h>

#if defined(__amd64__) || (defined(__i386__) && !defined(PC98))
#include <machine/cpufunc.h>
#include <machine/cputypes.h>
#include <machine/md_var.h>
#include <machine/specialreg.h>
#endif

#include <dev/random/randomdev.h>
#include <dev/random/randomdev_soft.h>

#if defined(__amd64__) || (defined(__i386__) && !defined(PC98))
#ifdef PADLOCK_RNG
extern struct random_systat random_nehemiah;
#endif
#ifdef RDRAND_RNG
extern struct random_systat random_ivy;
#endif
#endif

void
random_ident_hardware(struct random_systat *systat)
{

	/* Set default to software */
	*systat = random_yarrow;

	/* Then go looking for hardware */
#if defined(__amd64__) || (defined(__i386__) && !defined(PC98))
#ifdef PADLOCK_RNG
	if (via_feature_rng & VIA_HAS_RNG) {
		int enable;

		enable = 1;
		TUNABLE_INT_FETCH("hw.nehemiah_rng_enable", &enable);
		if (enable)
			*systat = random_nehemiah;
	}
#endif
#ifdef RDRAND_RNG
	if (cpu_feature2 & CPUID2_RDRAND) {
		int enable;

		enable = 1;
		TUNABLE_INT_FETCH("hw.ivy_rng_enable", &enable);
		if (enable)
			*systat = random_ivy;
	}
#endif
#endif
}