
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
static char sccsid[] = "@(#)subr.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/ktrace.h>

#include <stdio.h>

#include "ktrace.h"

void timevaladd(struct timeval *, struct timeval *);
void timevalsub(struct timeval *, struct timeval *);
void timevalfix(struct timeval *);

int
getpoints(char *s)
{
	int facs = 0;

	while (*s) {
		switch(*s) {
		case 'c':
			facs |= KTRFAC_SYSCALL | KTRFAC_SYSRET;
			break;
		case 'i':
			facs |= KTRFAC_GENIO;
			break;
		case 'f':
			facs |= KTRFAC_FAULT | KTRFAC_FAULTEND;
			break;
		case 'n':
			facs |= KTRFAC_NAMEI;
			break;
		case 'p':
			facs |= KTRFAC_CAPFAIL;
			break;
		case 's':
			facs |= KTRFAC_PSIG;
			break;
		case 't':
			facs |= KTRFAC_STRUCT;
			break;
		case 'u':
			facs |= KTRFAC_USER;
			break;
		case 'w':
			facs |= KTRFAC_CSW;
			break;
		case 'y':
			facs |= KTRFAC_SYSCTL;
			break;
		case '+':
			facs |= DEF_POINTS;
			break;
		default:
			return (-1);
		}
		s++;
	}
	return (facs);
}

void
timevaladd(struct timeval *t1, struct timeval *t2)
{
	t1->tv_sec += t2->tv_sec;
	t1->tv_usec += t2->tv_usec;
	timevalfix(t1);
}

void
timevalsub(struct timeval *t1, struct timeval *t2)
{
	t1->tv_sec -= t2->tv_sec;
	t1->tv_usec -= t2->tv_usec;
	timevalfix(t1);
}

void
timevalfix(struct timeval *t1)
{
	if (t1->tv_usec < 0) {
		t1->tv_sec--;
		t1->tv_usec += 1000000;
	}
	if (t1->tv_usec >= 1000000) {
		t1->tv_sec++;
		t1->tv_usec -= 1000000;
	}
}