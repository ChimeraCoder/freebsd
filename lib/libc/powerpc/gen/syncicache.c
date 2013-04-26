
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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <sys/param.h>
#if	defined(_KERNEL) || defined(_STANDALONE)
#include <sys/time.h>
#include <sys/proc.h>
#include <vm/vm.h>
#endif
#include <sys/sysctl.h>

#include <machine/cpu.h>
#include <machine/md_var.h>

#ifdef _STANDALONE
int cacheline_size = 32;
#endif

#if	!defined(_KERNEL) && !defined(_STANDALONE)
#include <stdlib.h>

int cacheline_size = 0;

static void getcachelinesize(void);

static void
getcachelinesize()
{
	static int	cachemib[] = { CTL_MACHDEP, CPU_CACHELINE };
	int		clen;

	clen = sizeof(cacheline_size);

	if (sysctl(cachemib, sizeof(cachemib) / sizeof(cachemib[0]),
	    &cacheline_size, &clen, NULL, 0) < 0 || !cacheline_size) {
		abort();
	}
}
#endif

void
__syncicache(void *from, int len)
{
	int	l, off;
	char	*p;

#if	!defined(_KERNEL) && !defined(_STANDALONE)
	if (!cacheline_size)
		getcachelinesize();
#endif	

	off = (u_int)from & (cacheline_size - 1);
	l = len += off;
	p = (char *)from - off;

	do {
		__asm __volatile ("dcbst 0,%0" :: "r"(p));
		p += cacheline_size;
	} while ((l -= cacheline_size) > 0);
	__asm __volatile ("sync");
	p = (char *)from - off;
	do {
		__asm __volatile ("icbi 0,%0" :: "r"(p));
		p += cacheline_size;
	} while ((len -= cacheline_size) > 0);
	__asm __volatile ("sync; isync");
}