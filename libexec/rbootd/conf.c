
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
#if 0
static const char sccsid[] = "@(#)conf.c	8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <sys/param.h>
#include <sys/time.h>

#include <stdio.h>
#include "defs.h"
#include "pathnames.h"

/*
**  Define (and possibly initialize) global variables here.
**
**  Caveat:
**	The maximum number of bootable files (`char *BootFiles[]') is
**	limited to C_MAXFILE (i.e. the maximum number of files that
**	can be spec'd in the configuration file).  This was done to
**	simplify the boot file search code.
*/

char	MyHost[MAXHOSTNAMELEN];			/* host name */
pid_t	MyPid;					/* process id */
int	DebugFlg = 0;				/* set true if debugging */
int	BootAny = 0;				/* set true if we boot anyone */

char	*ConfigFile = NULL;			/* configuration file */
char	*DfltConfig = _PATH_RBOOTDCONF;		/* default configuration file */
char	*PidFile = _PATH_RBOOTDPID;		/* file w/pid of server */
char	*BootDir = _PATH_RBOOTDLIB;		/* directory w/boot files */
char	*DbgFile = _PATH_RBOOTDDBG;		/* debug output file */

FILE	*DbgFp = NULL;				/* debug file pointer */
char	*IntfName = NULL;			/* intf we are attached to */

u_int16_t SessionID = 0;			/* generated session ID */

char	*BootFiles[C_MAXFILE];			/* list of boot files */

CLIENT	*Clients = NULL;			/* list of addrs we'll accept */
RMPCONN	*RmpConns = NULL;			/* list of active connections */

u_int8_t RmpMcastAddr[RMP_ADDRLEN] = RMP_ADDR;	/* RMP multicast address */