
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

/* a *lot* of ugly global definitions that really should be removed...
 */

#include "telnetd.h"

RCSID("$Id$");

/*
 * Telnet server variable declarations
 */
char	options[256];
char	do_dont_resp[256];
char	will_wont_resp[256];
int	linemode;	/* linemode on/off */
int	flowmode;	/* current flow control state */
int	restartany;	/* restart output on any character state */
#ifdef DIAGNOSTICS
int	diagnostic;	/* telnet diagnostic capabilities */
#endif /* DIAGNOSTICS */
int	require_otp;

slcfun	slctab[NSLC + 1];	/* slc mapping table */

char	terminaltype[41];

/*
 * I/O data buffers, pointers, and counters.
 */
char	ptyobuf[BUFSIZ+NETSLOP], *pfrontp, *pbackp;

char	netibuf[BUFSIZ], *netip;

char	netobuf[BUFSIZ+NETSLOP], *nfrontp, *nbackp;
char	*neturg;		/* one past last bye of urgent data */

int	pcc, ncc;

int	ourpty, net;
int	SYNCHing;		/* we are in TELNET SYNCH mode */

/*
 * The following are some clocks used to decide how to interpret
 * the relationship between various variables.
 */

struct clocks_t clocks;


/* whether to log unauthenticated login attempts */
int log_unauth;

/* do not print warning if connection is not encrypted */
int no_warn;

/*
 * This function appends data to nfrontp and advances nfrontp.
 */

int
output_data (const char *format, ...)
{
  va_list args;
  int remaining, ret;

  va_start(args, format);
  remaining = BUFSIZ - (nfrontp - netobuf);
  ret = vsnprintf (nfrontp,
		   remaining,
		   format,
		   args);
  nfrontp += min(ret, remaining-1);
  va_end(args);
  return ret;
}