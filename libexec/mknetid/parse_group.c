
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
static const char sccsid[] = "@(#)getgrent.c	8.2 (Berkeley) 3/21/94";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

/*
 * This is a slightly modified chunk of getgrent(3). All the YP support
 * and unneeded functions have been stripped out.
 */

#include <sys/types.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *_gr_fp;
static struct group _gr_group;
static int _gr_stayopen;
static int grscan(int, int);
static int start_gr(void);

#define	MAXGRP		200
static char *members[MAXGRP];
#define	MAXLINELENGTH	1024
static char line[MAXLINELENGTH];

struct group *
_getgrent(void)
{
	if (!_gr_fp && !start_gr()) {
		return NULL;
	}


	if (!grscan(0, 0))
		return(NULL);
	return(&_gr_group);
}

static int
start_gr(void)
{
	return 1;
}

int
_setgroupent(int stayopen)
{
	if (!start_gr())
		return(0);
	_gr_stayopen = stayopen;
	return(1);
}

int
_setgrent(void)
{
	return(_setgroupent(0));
}

void
_endgrent(void)
{
	if (_gr_fp) {
		(void)fclose(_gr_fp);
		_gr_fp = NULL;
	}
}

static int
grscan(int search, int gid)
{
	char *cp, **m;
	char *bp;
	for (;;) {
		if (!fgets(line, sizeof(line), _gr_fp))
			return(0);
		bp = line;
		/* skip lines that are too big */
		if (!strchr(line, '\n')) {
			int ch;

			while ((ch = getc(_gr_fp)) != '\n' && ch != EOF)
				;
			continue;
		}
		if ((_gr_group.gr_name = strsep(&bp, ":\n")) == NULL)
			break;
		if (_gr_group.gr_name[0] == '+')
			continue;
		if ((_gr_group.gr_passwd = strsep(&bp, ":\n")) == NULL)
			break;
		if (!(cp = strsep(&bp, ":\n")))
			continue;
		_gr_group.gr_gid = atoi(cp);
		if (search && _gr_group.gr_gid != gid)
			continue;
		cp = NULL;
		if (bp == NULL) /* !! Must check for this! */
			break;
		for (m = _gr_group.gr_mem = members;; bp++) {
			if (m == &members[MAXGRP - 1])
				break;
			if (*bp == ',') {
				if (cp) {
					*bp = '\0';
					*m++ = cp;
					cp = NULL;
				}
			} else if (*bp == '\0' || *bp == '\n' || *bp == ' ') {
				if (cp) {
					*bp = '\0';
					*m++ = cp;
			}
				break;
			} else if (cp == NULL)
				cp = bp;
		}
		*m = NULL;
		return(1);
	}
	/* NOTREACHED */
	return (0);
}