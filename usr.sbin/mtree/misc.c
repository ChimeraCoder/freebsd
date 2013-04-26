
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
static char sccsid[] = "@(#)misc.c	8.1 (Berkeley) 6/6/93";
#endif /*not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <fts.h>
#include <stdio.h>
#include <unistd.h>
#include "mtree.h"
#include "extern.h"

typedef struct _key {
	const char *name;			/* key name */
	u_int val;			/* value */

#define	NEEDVALUE	0x01
	u_int flags;
} KEY;

/* NB: the following table must be sorted lexically. */
static KEY keylist[] = {
	{"cksum",	F_CKSUM,	NEEDVALUE},
	{"flags",	F_FLAGS,	NEEDVALUE},
	{"gid",		F_GID,		NEEDVALUE},
	{"gname",	F_GNAME,	NEEDVALUE},
	{"ignore",	F_IGN,		0},
	{"link",	F_SLINK,	NEEDVALUE},
#ifdef MD5
	{"md5digest",	F_MD5,		NEEDVALUE},
#endif
	{"mode",	F_MODE,		NEEDVALUE},
	{"nlink",	F_NLINK,	NEEDVALUE},
	{"nochange",	F_NOCHANGE,	0},
	{"optional",	F_OPT,		0},
#ifdef RMD160
	{"ripemd160digest", F_RMD160,	NEEDVALUE},
#endif
#ifdef SHA1
	{"sha1digest",	F_SHA1,		NEEDVALUE},
#endif
#ifdef SHA256
	{"sha256digest",	F_SHA256,		NEEDVALUE},
#endif
	{"size",	F_SIZE,		NEEDVALUE},
	{"time",	F_TIME,		NEEDVALUE},
	{"type",	F_TYPE,		NEEDVALUE},
	{"uid",		F_UID,		NEEDVALUE},
	{"uname",	F_UNAME,	NEEDVALUE},
};

int keycompare(const void *, const void *);

u_int
parsekey(char *name, int *needvaluep)
{
	KEY *k, tmp;

	tmp.name = name;
	k = (KEY *)bsearch(&tmp, keylist, sizeof(keylist) / sizeof(KEY),
	    sizeof(KEY), keycompare);
	if (k == NULL)
		errx(1, "line %d: unknown keyword %s", lineno, name);

	if (needvaluep)
		*needvaluep = k->flags & NEEDVALUE ? 1 : 0;
	return (k->val);
}

int
keycompare(const void *a, const void *b)
{
	return (strcmp(((const KEY *)a)->name, ((const KEY *)b)->name));
}

char *
flags_to_string(u_long fflags)
{
	char *string;

	string = fflagstostr(fflags);
	if (string != NULL && *string == '\0') {
		free(string);
		string = strdup("none");
	}
	if (string == NULL)
		err(1, NULL);

	return string;
}