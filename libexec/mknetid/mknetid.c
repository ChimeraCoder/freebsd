
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

#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

#include <err.h>
#include <grp.h>
#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hash.h"

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#define LINSIZ 1024
#define OPSYS "unix"

/* Default location of group file. */
char *groupfile = _PATH_GROUP;
/* Default location of master.passwd file. */
char *passfile = _PATH_PASSWD;
/* Default location of hosts file. */
char *hostsfile = _PATH_HOSTS;
/* Default location of netid file */
char *netidfile = "/etc/netid";

/*
 * Stored hash table of 'reverse' group member database
 * which we will construct.
 */
struct member_entry *mtable[TABLESIZE];

/*
 * Dupe table: used to keep track of entries so we don't
 * print the same thing twice.
 */
struct member_entry *dtable[TABLESIZE];

extern struct group *_getgrent(void);
extern int _setgrent(void);
extern void _endgrent(void);

static void
usage(void)
{
	fprintf (stderr, "%s\n%s\n",
	"usage: mknetid [-q] [-g group_file] [-p passwd_file] [-h hosts_file]",
	"               [-n netid_file] [-d domain]");
	exit(1);
}

extern FILE *_gr_fp;

int
main(int argc, char *argv[])
{
	FILE *gfp, *pfp, *hfp, *nfp;
	char readbuf[LINSIZ];
	char writebuf[LINSIZ];
	struct group *gr;
	struct grouplist *glist;
	char *domain;
	int ch;
	gid_t i;
	char *ptr, *pidptr, *gidptr, *hptr;
	int quiet = 0;

	domain = NULL;
	while ((ch = getopt(argc, argv, "g:p:h:n:d:q")) != -1) {
		switch(ch) {
		case 'g':
			groupfile = optarg;
			break;
		case 'p':
			passfile = optarg;
			break;
		case 'h':
			hostsfile = optarg;
			break;
		case 'n':
			netidfile = optarg;
			break;
		case 'd':
			domain = optarg;
			break;
		case 'q':
			quiet++;
			break;
		default:
			usage();
			break;
		}
	}

	if (domain == NULL) {
		if (yp_get_default_domain(&domain))
			errx(1, "no domain name specified and default \
domain not set");
	}

	if ((gfp = fopen(groupfile, "r")) == NULL) {
		err(1, "%s", groupfile);
	}

	if ((pfp = fopen(passfile, "r")) == NULL) {
		err(1, "%s", passfile);
	}

	if ((hfp = fopen(hostsfile, "r")) == NULL) {
		err(1, "%s", hostsfile);
	}

	if ((nfp = fopen(netidfile, "r")) == NULL) {
		/* netid is optional -- just continue */
		nfp = NULL;
	}

	_gr_fp = gfp;

	/* Load all the group membership info into a hash table. */

	_setgrent();
	while((gr = _getgrent()) != NULL) {
		while(*gr->gr_mem) {
			mstore(mtable, *gr->gr_mem, gr->gr_gid, 0);
			gr->gr_mem++;
		}
	}

	fclose(gfp);
	_endgrent();

	/*
	 * Now parse the passwd database, spewing out the extra
	 * group information we just stored if necessary.
	 */
	while(fgets(readbuf, LINSIZ, pfp)) {
		/* Ignore comments: ^[ \t]*# */
		for (ptr = readbuf; *ptr != '\0'; ptr++)
			if (*ptr != ' ' && *ptr != '\t')
				break;
		if (*ptr == '#' || *ptr == '\0')
			continue;
		if ((ptr = strchr(readbuf, ':')) == NULL) {
			warnx("bad passwd file entry: %s", readbuf);
			continue;
		}
		*ptr = '\0';
		ptr++;
		if ((ptr = strchr(ptr, ':')) == NULL) {
			warnx("bad passwd file entry: %s", readbuf);
			continue;
		}
		*ptr = '\0';
		ptr++;
		pidptr = ptr;
		if ((ptr = strchr(ptr, ':')) == NULL) {
			warnx("bad passwd file entry: %s", readbuf);
			continue;
		}
		*ptr = '\0';
		ptr++;
		gidptr = ptr;
		if ((ptr = strchr(ptr, ':')) == NULL) {
			warnx("bad passwd file entry: %s", readbuf);
			continue;
		}
		*ptr = '\0';
		i = atol(gidptr);

		snprintf(writebuf, sizeof(writebuf), "%s.%s@%s", OPSYS,
							pidptr, domain);

		if (lookup(dtable, writebuf)) {
			if (!quiet)
				warnx("duplicate netid '%s.%s@%s' -- skipping",
						OPSYS, pidptr, domain);
			continue;
		} else {
			mstore(dtable, writebuf, 0, 1);
		}
		printf("%s.%s@%s %s:%s", OPSYS, pidptr, domain, pidptr, gidptr);
		if ((glist = lookup(mtable, (char *)&readbuf)) != NULL) {
			while(glist) {
				if (glist->groupid != i)
					printf(",%lu", (u_long)glist->groupid);
				glist = glist->next;
			}
		}
		printf ("\n");
	}	

	fclose(pfp);

	/*
	 * Now parse the hosts database (this part sucks).
	 */

	while ((ptr = fgets(readbuf, LINSIZ, hfp))) {
		if (*ptr == '#')
			continue;
		if (!(hptr = strpbrk(ptr, "#\n")))
			continue;
		*hptr = '\0';
		if (!(hptr = strpbrk(ptr, " \t")))
			continue;
		*hptr++ = '\0';
		ptr = hptr;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		if (!(hptr = strpbrk(ptr, " \t")))
			continue;
		*hptr++ = '\0';
		snprintf(writebuf, sizeof(writebuf), "%s.%s@%s", OPSYS,
								ptr, domain);
		if (lookup(dtable, (char *)&writebuf)) {
			if (!quiet)
				warnx("duplicate netid '%s' -- skipping",
								writebuf);
			continue;
		} else {
			mstore(dtable, (char *)&writebuf, 0, 1);
		}
		printf ("%s.%s@%s 0:%s\n", OPSYS, ptr, domain, ptr);
	}

	fclose(hfp);

	/*
	 * Lastly, copy out any extra information in the netid
	 * file. If it's not open, just ignore it: it's optional anyway.
	 */

	if (nfp != NULL) {
		while(fgets(readbuf, LINSIZ, nfp)) {
			if (readbuf[0] == '#')
				continue;
			if ((ptr = strpbrk((char*)&readbuf, " \t")) == NULL) {
				warnx("bad netid entry: '%s'", readbuf);
				continue;
			}

			writebuf[0] = *ptr;
			*ptr = '\0';
			if (lookup(dtable, (char *)&readbuf)) {
				if (!quiet)
					warnx("duplicate netid '%s' -- skipping",
								readbuf);
				continue;
			} else {
				mstore(dtable, (char *)&readbuf, 0, 1);
			}
			*ptr = writebuf[0];
			printf("%s",readbuf);
		}
		fclose(nfp);
	}

	exit(0);
}