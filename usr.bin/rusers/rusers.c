
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
#include <sys/socket.h>

#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpcsvc/rnusers.h>

#include <arpa/inet.h>

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <timeconv.h>
#include <unistd.h>

#define MAX_INT		0x7fffffff
#define HOST_WIDTH	20
#define LINE_WIDTH	15

static int longopt;
static int allopt;

static struct host_list {
	struct	host_list *next;
	struct	in_addr addr;
} *hosts;

static int
search_host(struct in_addr addr)
{
	struct host_list *hp;

	if (hosts == NULL)
		return (0);

	for (hp = hosts; hp != NULL; hp = hp->next) {
		if (hp->addr.s_addr == addr.s_addr)
			return (1);
	}
	return (0);
}

static void
remember_host(struct in_addr addr)
{
	struct host_list *hp;

	if ((hp = (struct host_list *)malloc(sizeof(struct host_list))) == NULL)
		errx(1, "no memory");
	hp->addr.s_addr = addr.s_addr;
	hp->next = hosts;
	hosts = hp;
}

static int
rusers_reply(caddr_t replyp, struct sockaddr_in *raddrp)
{
	u_int x;
	int idle;
	char date[32], idle_time[64], remote[64];
	struct hostent *hp;
	utmpidlearr *up, u;
	char *host;
	int days, hours, minutes, seconds;

	up = &u;
	memcpy(up, replyp, sizeof(*up));
	if (search_host(raddrp->sin_addr))
		return (0);

	if (!allopt && up->utmpidlearr_len == 0)
		return (0);

	hp = gethostbyaddr((char *)&raddrp->sin_addr.s_addr,
	    sizeof(struct in_addr), AF_INET);
	if (hp != NULL)
		host = hp->h_name;
	else
		host = inet_ntoa(raddrp->sin_addr);

	if (!longopt)
		printf("%-*s ", HOST_WIDTH, host);

	for (x = 0; x < up->utmpidlearr_len; x++) {
		time_t t = _int_to_time(up->utmpidlearr_val[x].ui_utmp.ut_time);
		strncpy(date, &(ctime(&t)[4]), sizeof(date) - 1);

		idle = up->utmpidlearr_val[x].ui_idle;
		sprintf(idle_time, "  :%02d", idle);
		if (idle == MAX_INT)
			strcpy(idle_time, "??");
		else if (idle == 0)
			strcpy(idle_time, "");
		else {
			seconds = idle;
			days = seconds / (60 * 60 * 24);
			seconds %= (60 * 60 * 24);
			hours = seconds / (60 * 60);
			seconds %= (60 * 60);
			minutes = seconds / 60;
			seconds %= 60;
			if (idle > 60)
				sprintf(idle_time, "%d:%02d", minutes, seconds);
			if (idle >= (60 * 60))
				sprintf(idle_time, "%d:%02d:%02d",
				    hours, minutes, seconds);
			if (idle >= (24 * 60 * 60))
				sprintf(idle_time, "%d days, %d:%02d:%02d",
				    days, hours, minutes, seconds);
		}

		strncpy(remote, up->utmpidlearr_val[x].ui_utmp.ut_host,
		    sizeof(remote) - 1);
		if (strlen(remote) != 0)
			sprintf(remote, "(%.16s)",
			    up->utmpidlearr_val[x].ui_utmp.ut_host);

		if (longopt)
			printf("%-8.8s %*s:%-*.*s %-12.12s  %6s %.18s\n",
			    up->utmpidlearr_val[x].ui_utmp.ut_name,
			    HOST_WIDTH, host, LINE_WIDTH, LINE_WIDTH,
			    up->utmpidlearr_val[x].ui_utmp.ut_line, date,
			    idle_time, remote );
		else
			printf("%s ",
			    up->utmpidlearr_val[x].ui_utmp.ut_name);
	}
	if (!longopt)
		putchar('\n');

	remember_host(raddrp->sin_addr);
	return (0);
}

static void
onehost(char *host)
{
	utmpidlearr up;
	CLIENT *rusers_clnt;
	struct sockaddr_in addr;
	struct hostent *hp;
	struct timeval tv;

	hp = gethostbyname(host);
	if (hp == NULL)
		errx(1, "unknown host \"%s\"", host);

	rusers_clnt = clnt_create(host, RUSERSPROG, RUSERSVERS_IDLE, "udp");
	if (rusers_clnt == NULL)
		errx(1, "%s", clnt_spcreateerror(""));

	bzero((char *)&up, sizeof(up));
	tv.tv_sec = 15;	/* XXX ?? */
	tv.tv_usec = 0;
	if (clnt_call(rusers_clnt, RUSERSPROC_NAMES, (xdrproc_t)xdr_void, NULL,
	    (xdrproc_t)xdr_utmpidlearr, &up, tv) != RPC_SUCCESS)
		errx(1, "%s", clnt_sperror(rusers_clnt, ""));
	memcpy(&addr.sin_addr.s_addr, hp->h_addr, sizeof(addr.sin_addr.s_addr));
	rusers_reply((caddr_t)&up, &addr);
	clnt_destroy(rusers_clnt);
}

static void
allhosts(void)
{
	utmpidlearr up;
	enum clnt_stat clnt_stat;

	bzero((char *)&up, sizeof(up));
	clnt_stat = clnt_broadcast(RUSERSPROG, RUSERSVERS_IDLE,
	    RUSERSPROC_NAMES, (xdrproc_t)xdr_void, NULL,
	    (xdrproc_t)xdr_utmpidlearr, (char *)&up,
	    (resultproc_t)rusers_reply);
	if (clnt_stat != RPC_SUCCESS && clnt_stat != RPC_TIMEDOUT)
		errx(1, "%s", clnt_sperrno(clnt_stat));
}

static void
usage(void)
{

	fprintf(stderr, "usage: rusers [-al] [host ...]\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "al")) != -1)
		switch (ch) {
		case 'a':
			allopt++;
			break;
		case 'l':
			longopt++;
			break;
		default:
			usage();
			/* NOTREACHED */
		}

	setlinebuf(stdout);
	if (argc == optind)
		allhosts();
	else {
		for (; optind < argc; optind++)
			(void)onehost(argv[optind]);
	}
	exit(0);
}