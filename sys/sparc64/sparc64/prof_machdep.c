
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

#ifdef GUPROF

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/gmon.h>
#include <sys/kernel.h>
#include <sys/sysctl.h>

#include <machine/profile.h>

int	cputime_bias;

/*
 * Return the time elapsed since the last call.  The units are machine-
 * dependent.
 * XXX: this is not SMP-safe.  It should use per-CPU variables; %tick can be
 * used though.
 */
int
cputime(void)
{
	u_long count;
	static u_long prev_count;
	int delta;

	count = rd(tick);
	delta = (int)(count - prev_count);
	prev_count = count;
	return (delta);
}

/*
 * The start and stop routines need not be here since we turn off profiling
 * before calling them.  They are here for convenience.
 */
void
startguprof(struct gmonparam *gp)
{

	gp->profrate = tick_freq;
	cputime_bias = 0;
	cputime();
}

void
stopguprof(struct gmonparam *gp)
{

	/* Nothing to do. */
}

#endif /* GUPROF */