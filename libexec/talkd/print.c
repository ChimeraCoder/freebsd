
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
static char sccsid[] = "@(#)print.c	8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

/* debug print routines */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <protocols/talkd.h>
#include <stdio.h>
#include <syslog.h>

#include "extern.h"

static	const char *types[] =
    { "leave_invite", "look_up", "delete", "announce" };
#define	NTYPES	(sizeof (types) / sizeof (types[0]))
static	const char *answers[] =
    { "success", "not_here", "failed", "machine_unknown", "permission_denied",
      "unknown_request", "badversion", "badaddr", "badctladdr" };
#define	NANSWERS	(sizeof (answers) / sizeof (answers[0]))

void
print_request(const char *cp, CTL_MSG *mp)
{
	const char *tp;
	char tbuf[80];

	if (mp->type > NTYPES) {
		(void)snprintf(tbuf, sizeof(tbuf), "type %d", mp->type);
		tp = tbuf;
	} else
		tp = types[mp->type];
	syslog(LOG_DEBUG, "%s: %s: id %lu, l_user %s, r_user %s, r_tty %s",
	    cp, tp, (long)mp->id_num, mp->l_name, mp->r_name, mp->r_tty);
}

void
print_response(const char *cp, CTL_RESPONSE *rp)
{
	const char *tp, *ap;
	char tbuf[80], abuf[80];

	if (rp->type > NTYPES) {
		(void)snprintf(tbuf, sizeof(tbuf), "type %d", rp->type);
		tp = tbuf;
	} else
		tp = types[rp->type];
	if (rp->answer > NANSWERS) {
		(void)snprintf(abuf, sizeof(abuf), "answer %d", rp->answer);
		ap = abuf;
	} else
		ap = answers[rp->answer];
	syslog(LOG_DEBUG, "%s: %s: %s, id %d", cp, tp, ap, ntohl(rp->id_num));
}