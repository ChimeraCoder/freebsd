
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

#include <config.h>

#include <signal.h>
#include "roken.h"

/*
 * We would like to always use this signal but there is a link error
 * on NEXTSTEP
 */
#if !defined(NeXT) && !defined(__APPLE__)
/*
 * Bugs:
 *
 * Do we need any extra hacks for SIGCLD and/or SIGCHLD?
 */

ROKEN_LIB_FUNCTION SigAction ROKEN_LIB_CALL
signal(int iSig, SigAction pAction)
{
    struct sigaction saNew, saOld;

    saNew.sa_handler = pAction;
    sigemptyset(&saNew.sa_mask);
    saNew.sa_flags = 0;

    if (iSig == SIGALRM)
	{
#ifdef SA_INTERRUPT
	    saNew.sa_flags |= SA_INTERRUPT;
#endif
	}
    else
	{
#ifdef SA_RESTART
	    saNew.sa_flags |= SA_RESTART;
#endif
	}

    if (sigaction(iSig, &saNew, &saOld) < 0)
	return(SIG_ERR);

    return(saOld.sa_handler);
}
#endif