
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

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)perlsfio.c	8.1 (Berkeley) 9/24/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <bitstring.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include "../common/common.h"

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "perl_extern.h"

/*
 * PUBLIC: #ifdef USE_SFIO
 */
#ifdef USE_SFIO

#define NIL(type)       ((type)0)

static int
sfnviwrite(f, buf, n, disc)
Sfio_t* f;      /* stream involved */
char*           buf;    /* buffer to read into */
int             n;      /* number of bytes to read */
Sfdisc_t*       disc;   /* discipline */        
{
	SCR *scrp;

	scrp = (SCR *)SvIV((SV*)SvRV(perl_get_sv("curscr", FALSE)));
	msgq(scrp, M_INFO, "%.*s", n, buf);
	return n;
}

/*
 * sfdcnewnvi --
 *	Create nvi discipline
 *
 * PUBLIC: Sfdisc_t* sfdcnewnvi __P((SCR*));
 */

Sfdisc_t *
sfdcnewnvi(scrp)
	SCR *scrp;
{
	Sfdisc_t*   disc;

	MALLOC(scrp, disc, Sfdisc_t*, sizeof(Sfdisc_t));
	if (!disc) return disc;

	disc->readf = (Sfread_f)NULL;
	disc->writef = sfnviwrite;
	disc->seekf = (Sfseek_f)NULL;
	disc->exceptf = (Sfexcept_f)NULL;
	return disc;
}

/*
 * PUBLIC: #endif
 */
#endif /* USE_SFIO */