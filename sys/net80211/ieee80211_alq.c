
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
#ifdef __FreeBSD__
__FBSDID("$FreeBSD$");
#endif

/*
 * net80211 fast-logging support, primarily for debugging.
 *
 * This implements a single debugging queue which includes
 * per-device enumeration where needed.
 */

#include "opt_wlan.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/endian.h>
#include <sys/kernel.h>
#include <sys/sysctl.h>
#include <sys/pcpu.h>
#include <sys/proc.h>
#include <sys/ucred.h>
#include <sys/alq.h>

#include <sys/socket.h>

#include <net/if.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <net/if_clone.h>
#include <net/if_media.h>
#include <net/if_types.h>

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_freebsd.h>
#include <net80211/ieee80211_alq.h>

static struct alq *ieee80211_alq;
static int ieee80211_alq_lost;
static int ieee80211_alq_logged;
static char ieee80211_alq_logfile[MAXPATHLEN] = "/tmp/net80211.log";
static unsigned int ieee80211_alq_qsize = 64*1024;

static int
ieee80211_alq_setlogging(int enable)
{
	int error;

	if (enable) {
		if (ieee80211_alq)
			alq_close(ieee80211_alq);

		error = alq_open(&ieee80211_alq,
		    ieee80211_alq_logfile,
		    curthread->td_ucred,
		    ALQ_DEFAULT_CMODE,
		    sizeof (struct ieee80211_alq_rec),
		    ieee80211_alq_qsize);
		ieee80211_alq_lost = 0;
		ieee80211_alq_logged = 0;
		printf("net80211: logging to %s enabled; "
		    "struct size %d bytes\n",
		    ieee80211_alq_logfile,
		    sizeof(struct ieee80211_alq_rec));
	} else {
		if (ieee80211_alq)
			alq_close(ieee80211_alq);
		ieee80211_alq = NULL;
		printf("net80211: logging disabled\n");
		error = 0;
	}
	return (error);
}

static int
sysctl_ieee80211_alq_log(SYSCTL_HANDLER_ARGS)
{
	int error, enable;

	enable = (ieee80211_alq != NULL);
	error = sysctl_handle_int(oidp, &enable, 0, req);
	if (error || !req->newptr)
		return (error);
	else
		return (ieee80211_alq_setlogging(enable));
}

SYSCTL_PROC(_net_wlan, OID_AUTO, alq, CTLTYPE_INT|CTLFLAG_RW,
	0, 0, sysctl_ieee80211_alq_log, "I", "Enable net80211 alq logging");
SYSCTL_INT(_net_wlan, OID_AUTO, alq_size, CTLFLAG_RW,
	&ieee80211_alq_qsize, 0, "In-memory log size (#records)");
SYSCTL_INT(_net_wlan, OID_AUTO, alq_lost, CTLFLAG_RW,
	&ieee80211_alq_lost, 0, "Debugging operations not logged");
SYSCTL_INT(_net_wlan, OID_AUTO, alq_logged, CTLFLAG_RW,
	&ieee80211_alq_logged, 0, "Debugging operations logged");

static struct ale *
ieee80211_alq_get(void)
{
	struct ale *ale;

	ale = alq_get(ieee80211_alq, ALQ_NOWAIT);
	if (!ale)
		ieee80211_alq_lost++;
	else
		ieee80211_alq_logged++;
	return ale;
}

void
ieee80211_alq_log(struct ieee80211vap *vap, uint8_t op, u_char *p, int l)
{
	struct ale *ale;
	struct ieee80211_alq_rec *r;

	if (ieee80211_alq == NULL)
		return;

	ale = ieee80211_alq_get();
	if (! ale)
		return;

	r = (struct ieee80211_alq_rec *) ale->ae_data;
	r->r_timestamp = htonl(ticks);
	r->r_version = 1;
	r->r_wlan = htons(vap->iv_ifp->if_dunit);
	r->r_op = op;
	r->r_threadid = htonl((uint32_t) curthread->td_tid);
	memcpy(&r->r_payload, p, MIN(l, sizeof(r->r_payload)));
	alq_post(ieee80211_alq, ale);
}