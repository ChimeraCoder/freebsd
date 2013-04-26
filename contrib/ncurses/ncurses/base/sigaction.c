
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

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-2003               *
 ****************************************************************************/

/* This file provides sigaction() emulation using sigvec() */
/* Use only if this is non POSIX system */

MODULE_ID("$Id: sigaction.c,v 1.14 2003/12/07 01:06:52 tom Exp $")

static int
_nc_sigaction(int sig, sigaction_t * sigact, sigaction_t * osigact)
{
    return sigvec(sig, sigact, osigact);
}

static int
_nc_sigemptyset(sigset_t * mask)
{
    *mask = 0;
    return 0;
}

static int
_nc_sigprocmask(int mode, sigset_t * mask, sigset_t * omask)
{
    sigset_t current = sigsetmask(0);

    if (omask)
	*omask = current;

    if (mode == SIG_BLOCK)
	current |= *mask;
    else if (mode == SIG_UNBLOCK)
	current &= ~*mask;
    else if (mode == SIG_SETMASK)
	current = *mask;

    sigsetmask(current);
    return 0;
}

static int
_nc_sigaddset(sigset_t * mask, int sig)
{
    *mask |= sigmask(sig);
    return 0;
}

/* not used in lib_tstp.c */
#if 0
static int
_nc_sigsuspend(sigset_t * mask)
{
    return sigpause(*mask);
}

static int
_nc_sigdelset(sigset_t * mask, int sig)
{
    *mask &= ~sigmask(sig);
    return 0;
}

static int
_nc_sigismember(sigset_t * mask, int sig)
{
    return (*mask & sigmask(sig)) != 0;
}
#endif