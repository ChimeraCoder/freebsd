
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
/*	$NetBSD: globals.c,v 1.3 1995/09/18 21:19:27 pk Exp $	*/
/*
 *	globals.c:
 *
 *	global variables should be separate, so nothing else
 *	must be included extraneously.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>

#include "stand.h"
#include "net.h"

u_char	bcea[6] = BA;			/* broadcast ethernet address */

char	rootpath[FNAME_SIZE] = "/";	/* root mount path */
char	bootfile[FNAME_SIZE];		/* bootp says to boot this */
char	hostname[FNAME_SIZE];		/* our hostname */
int	hostnamelen;
char	domainname[FNAME_SIZE];		/* our DNS domain */
int	domainnamelen;
char	ifname[IFNAME_SIZE];		/* name of interface (e.g. "le0") */
struct	in_addr myip;			/* my ip address */
struct	in_addr nameip;			/* DNS server ip address */
struct	in_addr rootip;			/* root ip address */
struct	in_addr swapip;			/* swap ip address */
struct	in_addr gateip;			/* swap ip address */
n_long	netmask = 0xffffff00;		/* subnet or net mask */
int	errno;				/* our old friend */