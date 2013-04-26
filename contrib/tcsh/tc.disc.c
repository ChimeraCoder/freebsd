
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
#include "sh.h"

RCSID("$tcsh: tc.disc.c,v 3.18 2011/01/09 16:25:29 christos Exp $")

#ifdef OREO
#include <compat.h>
#endif	/* OREO */

#include "ed.h"

static int add_discipline = 0;	/* Did we add a line discipline	 */

#if defined(IRIS4D) || defined(OREO) || defined(sonyrisc) || defined(__ANDROID__)
# define HAVE_DISC
# ifndef POSIX
static struct termio otermiob;
# else
static struct termios otermiob;
# endif /* POSIX */
#endif	/* IRIS4D || OREO */

#ifdef _IBMR2
# define HAVE_DISC
char    strPOSIX[] = "posix";
#endif	/* _IBMR2 */

#if !defined(HAVE_DISC) && defined(TIOCGETD) && defined(NTTYDISC)
static int oldisc;
#endif /* !HAVE_DISC && TIOCGETD && NTTYDISC */

int
/*ARGSUSED*/
setdisc(int f)
{
#ifdef IRIS4D
# ifndef POSIX
    struct termio termiob;
# else
    struct termios termiob;
# endif

    if (ioctl(f, TCGETA, (ioctl_t) & termiob) == 0) {
	otermiob = termiob;
#if (SYSVREL < 4) || !defined(IRIS4D)
	if (termiob.c_line != NTTYDISC || termiob.c_cc[VSWTCH] == 0) { /*}*/
	    termiob.c_line = NTTYDISC;
#else
	if (termiob.c_cc[VSWTCH] == 0) {
#endif
	    termiob.c_cc[VSWTCH] = CSWTCH;
	    if (ioctl(f, TCSETA, (ioctl_t) & termiob) != 0)
		return (-1);
	}
    }
    else
	return (-1);
    add_discipline = 1;
    return (0);
#endif /* IRIS4D */


#ifdef OREO
# ifndef POSIX
    struct termio termiob;
# else
    struct termios termiob;
# endif

    struct ltchars ltcbuf;

    if (ioctl(f, TCGETA, (ioctl_t) & termiob) == 0) {
	int comp = getcompat(COMPAT_BSDTTY);
	otermiob = termiob;
	if ((comp & COMPAT_BSDTTY) != COMPAT_BSDTTY) {
	    (void) setcompat(comp | COMPAT_BSDTTY);
	    if (ioctl(f, TIOCGLTC, (ioctl_t) & ltcbuf) != 0)
		xprintf(CGETS(21, 1, "Couldn't get local chars.\n"));
	    else {
		ltcbuf.t_suspc = CTL_ESC('\032');        /* ^Z */
		ltcbuf.t_dsuspc = CTL_ESC('\031');       /* ^Y */
		ltcbuf.t_rprntc = CTL_ESC('\022');       /* ^R */
		ltcbuf.t_flushc = CTL_ESC('\017');       /* ^O */
		ltcbuf.t_werasc = CTL_ESC('\027');       /* ^W */
		ltcbuf.t_lnextc = CTL_ESC('\026');       /* ^V */
		if (ioctl(f, TIOCSLTC, (ioctl_t) & ltcbuf) != 0)
		    xprintf(CGETS(21, 2, "Couldn't set local chars.\n"));
	    }
	    termiob.c_cc[VSWTCH] = '\0';
	    if (ioctl(f, TCSETAF, (ioctl_t) & termiob) != 0)
		return (-1);
	}
    }
    else
	return (-1);
    add_discipline = 1;
    return (0);
#endif				/* OREO */


#ifdef _IBMR2
    union txname tx;

    tx.tx_which = 0;

    if (ioctl(f, TXGETLD, (ioctl_t) & tx) == 0) {
	if (strcmp(tx.tx_name, strPOSIX) != 0)
	    if (ioctl(f, TXADDCD, (ioctl_t) strPOSIX) == 0) {
		add_discipline = 1;
		return (0);
	    }
	return (0);
    }
    else
	return (-1);
#endif	/* _IBMR2 */

#ifndef HAVE_DISC
# if defined(TIOCGETD) && defined(NTTYDISC)
    if (ioctl(f, TIOCGETD, (ioctl_t) & oldisc) == 0) {
	if (oldisc != NTTYDISC) {
	    int     ldisc = NTTYDISC;

	    if (ioctl(f, TIOCSETD, (ioctl_t) & ldisc) != 0)
		return (-1);
	    add_discipline = 1;
	}
	else
	    oldisc = -1;
	return (0);
    }
    else
	return (-1);
# else
    USE(f);
    return (0);
# endif	/* TIOCGETD && NTTYDISC */
#endif	/* !HAVE_DISC */
} /* end setdisc */


int
/*ARGSUSED*/
resetdisc(int f)
{
    if (add_discipline) {
	add_discipline = 0;
#if defined(OREO) || defined(IRIS4D)
	return (ioctl(f, TCSETAF, (ioctl_t) & otermiob));
#endif /* OREO || IRIS4D */

#ifdef _IBMR2
	return (ioctl(f, TXDELCD, (ioctl_t) strPOSIX));
#endif /* _IBMR2 */

#ifndef HAVE_DISC
# if defined(TIOCSETD) && defined(NTTYDISC)
	return (ioctl(f, TIOCSETD, (ioctl_t) & oldisc));
# endif /* TIOCSETD && NTTYDISC */
#endif /* !HAVE_DISC */
    }
    USE(f);
    return (0);
} /* end resetdisc */