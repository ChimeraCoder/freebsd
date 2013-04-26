
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

#if 0
#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1988, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)chroot.c	8.1 (Berkeley) 6/9/93";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <ctype.h>
#include <err.h>
#include <grp.h>
#include <limits.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(void);

int
main(int argc, char *argv[])
{
	struct group	*gp;
	struct passwd	*pw;
	char		*endp, *p, *user, *group, *grouplist;
	const char	*shell;
	gid_t		gid, *gidlist;
	uid_t		uid;
	int		ch, gids;
	long		ngroups_max;

	gid = 0;
	uid = 0;
	user = group = grouplist = NULL;
	while ((ch = getopt(argc, argv, "G:g:u:")) != -1) {
		switch(ch) {
		case 'u':
			user = optarg;
			if (*user == '\0')
				usage();
			break;
		case 'g':
			group = optarg;
			if (*group == '\0')
				usage();
			break;
		case 'G':
			grouplist = optarg;
			if (*grouplist == '\0')
				usage();
			break;
		case '?':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();

	if (group != NULL) {
		if (isdigit((unsigned char)*group)) {
			gid = (gid_t)strtoul(group, &endp, 0);
			if (*endp != '\0')
				goto getgroup;
		} else {
 getgroup:
			if ((gp = getgrnam(group)) != NULL)
				gid = gp->gr_gid;
			else
				errx(1, "no such group `%s'", group);
		}
	}

	ngroups_max = sysconf(_SC_NGROUPS_MAX) + 1;
	if ((gidlist = malloc(sizeof(gid_t) * ngroups_max)) == NULL)
		err(1, "malloc");
	for (gids = 0;
	    (p = strsep(&grouplist, ",")) != NULL && gids < ngroups_max; ) {
		if (*p == '\0')
			continue;

		if (isdigit((unsigned char)*p)) {
			gidlist[gids] = (gid_t)strtoul(p, &endp, 0);
			if (*endp != '\0')
				goto getglist;
		} else {
 getglist:
			if ((gp = getgrnam(p)) != NULL)
				gidlist[gids] = gp->gr_gid;
			else
				errx(1, "no such group `%s'", p);
		}
		gids++;
	}
	if (p != NULL && gids == ngroups_max)
		errx(1, "too many supplementary groups provided");

	if (user != NULL) {
		if (isdigit((unsigned char)*user)) {
			uid = (uid_t)strtoul(user, &endp, 0);
			if (*endp != '\0')
				goto getuser;
		} else {
 getuser:
			if ((pw = getpwnam(user)) != NULL)
				uid = pw->pw_uid;
			else
				errx(1, "no such user `%s'", user);
		}
	}

	if (chdir(argv[0]) == -1 || chroot(".") == -1)
		err(1, "%s", argv[0]);

	if (gids && setgroups(gids, gidlist) == -1)
		err(1, "setgroups");
	if (group && setgid(gid) == -1)
		err(1, "setgid");
	if (user && setuid(uid) == -1)
		err(1, "setuid");

	if (argv[1]) {
		execvp(argv[1], &argv[1]);
		err(1, "%s", argv[1]);
	}

	if (!(shell = getenv("SHELL")))
		shell = _PATH_BSHELL;
	execlp(shell, shell, "-i", (char *)NULL);
	err(1, "%s", shell);
	/* NOTREACHED */
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: chroot [-g group] [-G group,group,...] "
	    "[-u user] newroot [command]\n");
	exit(1);
}