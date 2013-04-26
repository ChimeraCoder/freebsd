
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#include "ipfw2.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <fcntl.h>

#include <net/if.h>		/* IFNAMSIZ */
#include <net/pfvar.h>
#include <netinet/in.h>	/* in_addr */
#include <netinet/ip_fw.h>

/*
 * Map between current altq queue id numbers and names.
 */
static TAILQ_HEAD(, pf_altq) altq_entries =
	TAILQ_HEAD_INITIALIZER(altq_entries);

void
altq_set_enabled(int enabled)
{
	int pffd;

	pffd = open("/dev/pf", O_RDWR);
	if (pffd == -1)
		err(EX_UNAVAILABLE,
		    "altq support opening pf(4) control device");
	if (enabled) {
		if (ioctl(pffd, DIOCSTARTALTQ) != 0 && errno != EEXIST)
			err(EX_UNAVAILABLE, "enabling altq");
	} else {
		if (ioctl(pffd, DIOCSTOPALTQ) != 0 && errno != ENOENT)
			err(EX_UNAVAILABLE, "disabling altq");
	}
	close(pffd);
}

static void
altq_fetch(void)
{
	struct pfioc_altq pfioc;
	struct pf_altq *altq;
	int pffd;
	unsigned int mnr;
	static int altq_fetched = 0;

	if (altq_fetched)
		return;
	altq_fetched = 1;
	pffd = open("/dev/pf", O_RDONLY);
	if (pffd == -1) {
		warn("altq support opening pf(4) control device");
		return;
	}
	bzero(&pfioc, sizeof(pfioc));
	if (ioctl(pffd, DIOCGETALTQS, &pfioc) != 0) {
		warn("altq support getting queue list");
		close(pffd);
		return;
	}
	mnr = pfioc.nr;
	for (pfioc.nr = 0; pfioc.nr < mnr; pfioc.nr++) {
		if (ioctl(pffd, DIOCGETALTQ, &pfioc) != 0) {
			if (errno == EBUSY)
				break;
			warn("altq support getting queue list");
			close(pffd);
			return;
		}
		if (pfioc.altq.qid == 0)
			continue;
		altq = safe_calloc(1, sizeof(*altq));
		*altq = pfioc.altq;
		TAILQ_INSERT_TAIL(&altq_entries, altq, entries);
	}
	close(pffd);
}

u_int32_t
altq_name_to_qid(const char *name)
{
	struct pf_altq *altq;

	altq_fetch();
	TAILQ_FOREACH(altq, &altq_entries, entries)
		if (strcmp(name, altq->qname) == 0)
			break;
	if (altq == NULL)
		errx(EX_DATAERR, "altq has no queue named `%s'", name);
	return altq->qid;
}

static const char *
altq_qid_to_name(u_int32_t qid)
{
	struct pf_altq *altq;

	altq_fetch();
	TAILQ_FOREACH(altq, &altq_entries, entries)
		if (qid == altq->qid)
			break;
	if (altq == NULL)
		return NULL;
	return altq->qname;
}

void
print_altq_cmd(ipfw_insn_altq *altqptr)
{
	if (altqptr) {
		const char *qname;

		qname = altq_qid_to_name(altqptr->qid);
		if (qname == NULL)
			printf(" altq ?<%u>", altqptr->qid);
		else
			printf(" altq %s", qname);
	}
}