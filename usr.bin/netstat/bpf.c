
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

#include <sys/types.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <sys/user.h>

#include <net/if.h>
#include <net/if_var.h>
#include <net/bpf.h>
#include <net/bpfdesc.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "netstat.h"

/* print bpf stats */

static char *
bpf_pidname(pid_t pid)
{
	struct kinfo_proc newkp;
	int error, mib[4];
	size_t size;

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = pid;
	size = sizeof(newkp);
	error = sysctl(mib, 4, &newkp, &size, NULL, 0);
	if (error < 0) {
		warn("kern.proc.pid failed");
		return (strdup("??????"));
	}
	return (strdup(newkp.ki_comm));
}

static void
bpf_flags(struct xbpf_d *bd, char *flagbuf)
{

	*flagbuf++ = bd->bd_promisc ? 'p' : '-';
	*flagbuf++ = bd->bd_immediate ? 'i' : '-';
	*flagbuf++ = bd->bd_hdrcmplt ? '-' : 'f';
	*flagbuf++ = (bd->bd_direction == BPF_D_IN) ? '-' :
	    ((bd->bd_direction == BPF_D_OUT) ? 'o' : 's');
	*flagbuf++ = bd->bd_feedback ? 'b' : '-';
	*flagbuf++ = bd->bd_async ? 'a' : '-';
	*flagbuf++ = bd->bd_locked ? 'l' : '-';
	*flagbuf++ = '\0';
}

void
bpf_stats(char *ifname)
{
	struct xbpf_d *d, *bd, zerostat;
	char *pname, flagbuf[12];
	size_t size;

	if (zflag) {
		bzero(&zerostat, sizeof(zerostat));
		if (sysctlbyname("net.bpf.stats", NULL, NULL,
		    &zerostat, sizeof(zerostat)) < 0)
			warn("failed to zero bpf counters");
		return;
	}
	if (sysctlbyname("net.bpf.stats", NULL, &size,
	    NULL, 0) < 0) {
		warn("net.bpf.stats");
		return;
	}
	if (size == 0)
		return;
	bd = malloc(size);
	if (bd == NULL) {
		warn("malloc failed");
		return;
	}
	if (sysctlbyname("net.bpf.stats", bd, &size,
	    NULL, 0) < 0) {
		warn("net.bpf.stats");
		free(bd);
		return;
	}
	(void) printf("%5s %6s %7s %9s %9s %9s %5s %5s %s\n",
	    "Pid", "Netif", "Flags", "Recv", "Drop", "Match", "Sblen",
	    "Hblen", "Command");
	for (d = &bd[0]; d < &bd[size / sizeof(*d)]; d++) {
		if (d->bd_structsize != sizeof(*d)) {
			warnx("bpf_stats_extended: version mismatch");
			return;
		}
		if (ifname && strcmp(ifname, d->bd_ifname) != 0)
			continue;
		bpf_flags(d, flagbuf);
		pname = bpf_pidname(d->bd_pid);
		(void) printf("%5d %6s %7s %9ju %9ju %9ju %5d %5d %s\n",
		    d->bd_pid, d->bd_ifname, flagbuf,
		    d->bd_rcount, d->bd_dcount, d->bd_fcount,
		    d->bd_slen, d->bd_hlen, pname);
		free(pname);
	}
	free(bd);
}