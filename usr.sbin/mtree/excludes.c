
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
#include <sys/time.h>		/* XXX for mtree.h */
#include <sys/queue.h>

#include <err.h>
#include <fnmatch.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>

#include "mtree.h"		/* XXX for extern.h */
#include "extern.h"

/*
 * We're assuming that there won't be a whole lot of excludes,
 * so it's OK to use a stupid algorithm.
 */
struct exclude {
	LIST_ENTRY(exclude) link;
	const char *glob;
	int pathname;
};
static LIST_HEAD(, exclude) excludes;

void
init_excludes(void)
{
	LIST_INIT(&excludes);
}

void
read_excludes_file(const char *name)
{
	FILE *fp;
	char *line, *str;
	struct exclude *e;
	size_t len;

	fp = fopen(name, "r");
	if (fp == 0)
		err(1, "%s", name);

	while ((line = fgetln(fp, &len)) != 0) {
		if (line[len - 1] == '\n')
			len--;
		if (len == 0)
			continue;

		str = malloc(len + 1);
		e = malloc(sizeof *e);
		if (str == 0 || e == 0)
			errx(1, "memory allocation error");
		e->glob = str;
		memcpy(str, line, len);
		str[len] = '\0';
		if (strchr(str, '/'))
			e->pathname = 1;
		else
			e->pathname = 0;
		LIST_INSERT_HEAD(&excludes, e, link);
	}
	fclose(fp);
}

int
check_excludes(const char *fname, const char *path)
{
	struct exclude *e;

	/* fnmatch(3) has a funny return value convention... */
#define MATCH(g, n) (fnmatch((g), (n), FNM_PATHNAME) == 0)

	LIST_FOREACH(e, &excludes, link) {
		if ((e->pathname && MATCH(e->glob, path))
		    || MATCH(e->glob, fname))
			return 1;
	}
	return 0;
}