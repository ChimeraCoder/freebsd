
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
static char sccsid[] = "@(#)ls.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fts.h>
#include <grp.h>
#include <inttypes.h>
#include <langinfo.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "find.h"

/* Derived from the print routines in the ls(1) source code. */

static void printlink(char *);
static void printtime(time_t);

void
printlong(char *name, char *accpath, struct stat *sb)
{
	char modep[15];

	(void)printf("%6ju %8"PRId64" ", (uintmax_t)sb->st_ino, sb->st_blocks);
	(void)strmode(sb->st_mode, modep);
	(void)printf("%s %3u %-*s %-*s ", modep, sb->st_nlink, MAXLOGNAME - 1,
	    user_from_uid(sb->st_uid, 0), MAXLOGNAME - 1,
	    group_from_gid(sb->st_gid, 0));

	if (S_ISCHR(sb->st_mode) || S_ISBLK(sb->st_mode))
		(void)printf("%#8jx ", (uintmax_t)sb->st_rdev);
	else
		(void)printf("%8"PRId64" ", sb->st_size);
	printtime(sb->st_mtime);
	(void)printf("%s", name);
	if (S_ISLNK(sb->st_mode))
		printlink(accpath);
	(void)putchar('\n');
}

static void
printtime(time_t ftime)
{
	char longstring[80];
	static time_t lnow;
	const char *format;
	static int d_first = -1;

	if (d_first < 0)
		d_first = (*nl_langinfo(D_MD_ORDER) == 'd');
	if (lnow == 0)
		lnow = time(NULL);

#define	SIXMONTHS	((365 / 2) * 86400)
	if (ftime + SIXMONTHS > lnow && ftime < lnow + SIXMONTHS)
		/* mmm dd hh:mm || dd mmm hh:mm */
		format = d_first ? "%e %b %R " : "%b %e %R ";
	else
		/* mmm dd  yyyy || dd mmm  yyyy */
		format = d_first ? "%e %b  %Y " : "%b %e  %Y ";
	strftime(longstring, sizeof(longstring), format, localtime(&ftime));
	fputs(longstring, stdout);
}

static void
printlink(char *name)
{
	int lnklen;
	char path[MAXPATHLEN];

	if ((lnklen = readlink(name, path, MAXPATHLEN - 1)) == -1) {
		warn("%s", name);
		return;
	}
	path[lnklen] = '\0';
	(void)printf(" -> %s", path);
}