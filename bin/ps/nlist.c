
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)nlist.c	8.4 (Berkeley) 4/2/94";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/sysctl.h>

#include <stddef.h>

#include "ps.h"

fixpt_t	ccpu;				/* kernel _ccpu variable */
int	nlistread;			/* if nlist already read. */
unsigned long	mempages;		/* number of pages of phys. memory */
int	fscale;				/* kernel _fscale variable */

int
donlist(void)
{
	size_t oldlen;

	oldlen = sizeof(ccpu);
	if (sysctlbyname("kern.ccpu", &ccpu, &oldlen, NULL, 0) == -1)
		return (1);
	oldlen = sizeof(fscale);
	if (sysctlbyname("kern.fscale", &fscale, &oldlen, NULL, 0) == -1)
		return (1);
	oldlen = sizeof(mempages);
	if (sysctlbyname("hw.availpages", &mempages, &oldlen, NULL, 0) == -1)
		return (1);
	nlistread = 1;
	return (0);
}