
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
static char sccsid[] = "@(#)regfree.c	8.3 (Berkeley) 3/20/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <regex.h>
#include <wchar.h>
#include <wctype.h>

#include "utils.h"
#include "regex2.h"

/*
 - regfree - free everything
 = extern void regfree(regex_t *);
 */
void
regfree(regex_t *preg)
{
	struct re_guts *g;
	int i;

	if (preg->re_magic != MAGIC1)	/* oops */
		return;			/* nice to complain, but hard */

	g = preg->re_g;
	if (g == NULL || g->magic != MAGIC2)	/* oops again */
		return;
	preg->re_magic = 0;		/* mark it invalid */
	g->magic = 0;			/* mark it invalid */

	if (g->strip != NULL)
		free((char *)g->strip);
	if (g->sets != NULL) {
		for (i = 0; i < g->ncsets; i++) {
			free(g->sets[i].ranges);
			free(g->sets[i].wides);
			free(g->sets[i].types);
		}
		free((char *)g->sets);
	}
	if (g->must != NULL)
		free(g->must);
	if (g->charjump != NULL)
		free(&g->charjump[CHAR_MIN]);
	if (g->matchjump != NULL)
		free(g->matchjump);
	free((char *)g);
}