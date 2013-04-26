
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
static const char copyright[] =
"@(#) Copyright (c) 1980, 1988, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)from.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <ctype.h>
#include <err.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>

int match(const char *, const char *);
static void usage(void);

int
main(int argc, char **argv)
{
	FILE *mbox;
	struct passwd *pwd;
	int ch, count, newline;
	const char *file;
	char *sender, *p;
#if MAXPATHLEN > BUFSIZ
	char buf[MAXPATHLEN];
#else
	char buf[BUFSIZ];
#endif

	file = sender = NULL;
	count = -1;
	while ((ch = getopt(argc, argv, "cf:s:")) != -1)
		switch (ch) {
		case 'c':
			count = 0;
			break;
		case 'f':
			file = optarg;
			break;
		case 's':
			sender = optarg;
			for (p = sender; *p; ++p)
				if (isupper(*p))
					*p = tolower(*p);
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (file == NULL) {
		if (argc) {
			(void)snprintf(buf, sizeof(buf), "%s/%s", _PATH_MAILDIR, *argv);
			file  = buf;
		} else {
			if (!(file = getenv("MAIL"))) {
				if (!(pwd = getpwuid(getuid())))
					errx(1, "no password file entry for you");
				file = pwd->pw_name;
				(void)snprintf(buf, sizeof(buf),
				    "%s/%s", _PATH_MAILDIR, file);
				file = buf;
			}
		}
	}

	/* read from stdin */
	if (strcmp(file, "-") == 0) {
		mbox = stdin;
	} 
	else if ((mbox = fopen(file, "r")) == NULL) {
		errx(1, "can't read %s", file);
	}
	for (newline = 1; fgets(buf, sizeof(buf), mbox);) {
		if (*buf == '\n') {
			newline = 1;
			continue;
		}
		if (newline && !strncmp(buf, "From ", 5) &&
		    (!sender || match(buf + 5, sender))) {
			if (count != -1)
				count++;
			else
				printf("%s", buf);
		}
		newline = 0;
	}
	if (count != -1)
		printf("There %s %d message%s in your incoming mailbox.\n",
		    count == 1 ? "is" : "are", count, count == 1 ? "" : "s"); 
	fclose(mbox);
	exit(0);
}

static void
usage(void)
{
	fprintf(stderr, "usage: from [-c] [-f file] [-s sender] [user]\n");
	exit(1);
}

int
match(const char *line, const char *sender)
{
	char ch, pch, first;
	const char *p, *t;

	for (first = *sender++;;) {
		if (isspace(ch = *line))
			return(0);
		++line;
		if (isupper(ch))
			ch = tolower(ch);
		if (ch != first)
			continue;
		for (p = sender, t = line;;) {
			if (!(pch = *p++))
				return(1);
			if (isupper(ch = *t++))
				ch = tolower(ch);
			if (ch != pch)
				break;
		}
	}
	/* NOTREACHED */
}