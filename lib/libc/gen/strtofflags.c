
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)stat_flags.c	8.1 (Berkeley) 5/31/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define longestflaglen	12
static struct {
	char name[longestflaglen + 1];
	char invert;
	u_long flag;
} const mapping[] = {
	/* shorter names per flag first, all prefixed by "no" */
	{ "nosappnd",		0, SF_APPEND	},
	{ "nosappend",		0, SF_APPEND	},
	{ "noarch",		0, SF_ARCHIVED	},
	{ "noarchived",		0, SF_ARCHIVED	},
	{ "noschg",		0, SF_IMMUTABLE	},
	{ "noschange",		0, SF_IMMUTABLE	},
	{ "nosimmutable",	0, SF_IMMUTABLE	},
	{ "nosunlnk",		0, SF_NOUNLINK	},
	{ "nosunlink",		0, SF_NOUNLINK	},
#ifdef SF_SNAPSHOT
	{ "nosnapshot",		0, SF_SNAPSHOT	},
#endif
	{ "nouappnd",		0, UF_APPEND	},
	{ "nouappend",		0, UF_APPEND	},
	{ "nouchg",		0, UF_IMMUTABLE	},
	{ "nouchange",		0, UF_IMMUTABLE	},
	{ "nouimmutable",	0, UF_IMMUTABLE	},
	{ "nodump",		1, UF_NODUMP	},
	{ "noopaque",		0, UF_OPAQUE	},
	{ "nouunlnk",		0, UF_NOUNLINK	},
	{ "nouunlink",		0, UF_NOUNLINK	}
};
#define nmappings	(sizeof(mapping) / sizeof(mapping[0]))

/*
 * fflagstostr --
 *	Convert file flags to a comma-separated string.  If no flags
 *	are set, return the empty string.
 */
char *
fflagstostr(flags)
	u_long flags;
{
	char *string;
	const char *sp;
	char *dp;
	u_long setflags;
	int i;

	if ((string = (char *)malloc(nmappings * (longestflaglen + 1))) == NULL)
		return (NULL);

	setflags = flags;
	dp = string;
	for (i = 0; i < nmappings; i++) {
		if (setflags & mapping[i].flag) {
			if (dp > string)
				*dp++ = ',';
			for (sp = mapping[i].invert ? mapping[i].name :
			    mapping[i].name + 2; *sp; *dp++ = *sp++) ;
			setflags &= ~mapping[i].flag;
		}
	}
	*dp = '\0';
	return (string);
}

/*
 * strtofflags --
 *	Take string of arguments and return file flags.  Return 0 on
 *	success, 1 on failure.  On failure, stringp is set to point
 *	to the offending token.
 */
int
strtofflags(stringp, setp, clrp)
	char **stringp;
	u_long *setp, *clrp;
{
	char *string, *p;
	int i;

	if (setp)
		*setp = 0;
	if (clrp)
		*clrp = 0;
	string = *stringp;
	while ((p = strsep(&string, "\t ,")) != NULL) {
		*stringp = p;
		if (*p == '\0')
			continue;
		for (i = 0; i < nmappings; i++) {
			if (strcmp(p, mapping[i].name + 2) == 0) {
				if (mapping[i].invert) {
					if (clrp)
						*clrp |= mapping[i].flag;
				} else {
					if (setp)
						*setp |= mapping[i].flag;
				}
				break;
			} else if (strcmp(p, mapping[i].name) == 0) {
				if (mapping[i].invert) {
					if (setp)
						*setp |= mapping[i].flag;
				} else {
					if (clrp)
						*clrp |= mapping[i].flag;
				}
				break;
			}
		}
		if (i == nmappings)
			return 1;
	}
	return 0;
}