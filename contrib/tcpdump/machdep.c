
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
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/tcpdump/machdep.c,v 1.13 2003-12-15 03:53:21 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * XXX - all we need, on platforms other than DEC OSF/1 (a/k/a Digital UNIX,
 * a/k/a Tru64 UNIX), is "size_t", which is a standard C type; what do we
 * need to do to get it defined?  This is clearly wrong, as we shouldn't
 * have to include UNIX or Windows system header files to get it.
 */
#include <tcpdump-stdinc.h>

#ifndef HAVE___ATTRIBUTE__
#define __attribute__(x)
#endif /* HAVE___ATTRIBUTE__ */

#ifdef __osf__
#include <sys/sysinfo.h>
#include <sys/proc.h>

#if !defined(HAVE_SNPRINTF)
int snprintf(char *, size_t, const char *, ...)
     __attribute__((format(printf, 3, 4)));
#endif /* !defined(HAVE_SNPRINTF) */
#endif /* __osf__ */

#include "machdep.h"

int
abort_on_misalignment(char *ebuf _U_, size_t ebufsiz _U_)
{
#ifdef __osf__
	static int buf[2] = { SSIN_UACPROC, UAC_SIGBUS };

	if (setsysinfo(SSI_NVPAIRS, (caddr_t)buf, 1, 0, 0) < 0) {
		(void)snprintf(ebuf, ebufsiz, "setsysinfo: errno %d", errno);
		return (-1);
	}
#endif
	return (0);
}