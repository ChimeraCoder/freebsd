
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
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <sys/queue.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ifconfig.h"

static void
list_cloners(void)
{
	struct if_clonereq ifcr;
	char *cp, *buf;
	int idx;
	int s;

	s = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (s == -1)
		err(1, "socket(AF_LOCAL,SOCK_DGRAM)");

	memset(&ifcr, 0, sizeof(ifcr));

	if (ioctl(s, SIOCIFGCLONERS, &ifcr) < 0)
		err(1, "SIOCIFGCLONERS for count");

	buf = malloc(ifcr.ifcr_total * IFNAMSIZ);
	if (buf == NULL)
		err(1, "unable to allocate cloner name buffer");

	ifcr.ifcr_count = ifcr.ifcr_total;
	ifcr.ifcr_buffer = buf;

	if (ioctl(s, SIOCIFGCLONERS, &ifcr) < 0)
		err(1, "SIOCIFGCLONERS for names");

	/*
	 * In case some disappeared in the mean time, clamp it down.
	 */
	if (ifcr.ifcr_count > ifcr.ifcr_total)
		ifcr.ifcr_count = ifcr.ifcr_total;

	for (cp = buf, idx = 0; idx < ifcr.ifcr_count; idx++, cp += IFNAMSIZ) {
		if (idx > 0)
			putchar(' ');
		printf("%s", cp);
	}

	putchar('\n');
	free(buf);
}

struct clone_defcb {
	char ifprefix[IFNAMSIZ];
	clone_callback_func *clone_cb;
	SLIST_ENTRY(clone_defcb) next;
};

static SLIST_HEAD(, clone_defcb) clone_defcbh =
   SLIST_HEAD_INITIALIZER(clone_defcbh);

void
clone_setdefcallback(const char *ifprefix, clone_callback_func *p)
{
	struct clone_defcb *dcp;

	dcp = malloc(sizeof(*dcp));
	strlcpy(dcp->ifprefix, ifprefix, IFNAMSIZ-1);
	dcp->clone_cb = p;
	SLIST_INSERT_HEAD(&clone_defcbh, dcp, next);
}

/*
 * Do the actual clone operation.  Any parameters must have been
 * setup by now.  If a callback has been setup to do the work
 * then defer to it; otherwise do a simple create operation with
 * no parameters.
 */
static void
ifclonecreate(int s, void *arg)
{
	struct ifreq ifr;
	struct clone_defcb *dcp;
	clone_callback_func *clone_cb = NULL;

	memset(&ifr, 0, sizeof(ifr));
	(void) strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

	if (clone_cb == NULL) {
		/* Try to find a default callback */
		SLIST_FOREACH(dcp, &clone_defcbh, next) {
			if (strncmp(dcp->ifprefix, ifr.ifr_name,
			    strlen(dcp->ifprefix)) == 0) {
				clone_cb = dcp->clone_cb;
				break;
			}
		}
	}
	if (clone_cb == NULL) {
		/* NB: no parameters */
		if (ioctl(s, SIOCIFCREATE2, &ifr) < 0)
			err(1, "SIOCIFCREATE2");
	} else {
		clone_cb(s, &ifr);
	}

	/*
	 * If we get a different name back than we put in, print it.
	 */
	if (strncmp(name, ifr.ifr_name, sizeof(name)) != 0) {
		strlcpy(name, ifr.ifr_name, sizeof(name));
		printf("%s\n", name);
	}
}

static
DECL_CMD_FUNC(clone_create, arg, d)
{
	callback_register(ifclonecreate, NULL);
}

static
DECL_CMD_FUNC(clone_destroy, arg, d)
{
	(void) strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCIFDESTROY, &ifr) < 0)
		err(1, "SIOCIFDESTROY");
}

static struct cmd clone_cmds[] = {
	DEF_CLONE_CMD("create",	0,	clone_create),
	DEF_CMD("destroy",	0,	clone_destroy),
	DEF_CLONE_CMD("plumb",	0,	clone_create),
	DEF_CMD("unplumb",	0,	clone_destroy),
};

static void
clone_Copt_cb(const char *optarg __unused)
{
	list_cloners();
	exit(0);
}
static struct option clone_Copt = { .opt = "C", .opt_usage = "[-C]", .cb = clone_Copt_cb };

static __constructor void
clone_ctor(void)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	size_t i;

	for (i = 0; i < N(clone_cmds);  i++)
		cmd_register(&clone_cmds[i]);
	opt_register(&clone_Copt);
#undef N
}