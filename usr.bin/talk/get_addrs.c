
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

#include <sys/cdefs.h>

__FBSDID("$FreeBSD$");

#ifndef lint
static const char sccsid[] = "@(#)get_addrs.c	8.1 (Berkeley) 6/6/93";
#endif

#include <err.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "talk.h"
#include "talk_ctl.h"

void
get_addrs(const char *my_machine_name __unused, const char *his_machine_name)
{
	struct hostent *hp;
	struct servent *sp;

	msg.pid = htonl(getpid());

	hp = gethostbyname(his_machine_name);
	if (hp == NULL)
		errx(1, "%s: %s", his_machine_name, hstrerror(h_errno));
	bcopy(hp->h_addr, (char *) &his_machine_addr, hp->h_length);
	if (get_iface(&his_machine_addr, &my_machine_addr) == -1)
		err(1, "failed to find my interface address");
	/* find the server's port */
	sp = getservbyname("ntalk", "udp");
	if (sp == 0)
		errx(1, "ntalk/udp: service is not registered");
	daemon_port = sp->s_port;
}