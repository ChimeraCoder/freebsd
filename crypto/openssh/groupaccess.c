
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

#include "includes.h"

#include <sys/types.h>
#include <sys/param.h>

#include <grp.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "xmalloc.h"
#include "groupaccess.h"
#include "match.h"
#include "log.h"

static int ngroups;
static char **groups_byname;

/*
 * Initialize group access list for user with primary (base) and
 * supplementary groups.  Return the number of groups in the list.
 */
int
ga_init(const char *user, gid_t base)
{
	gid_t *groups_bygid;
	int i, j;
	struct group *gr;

	if (ngroups > 0)
		ga_free();

	ngroups = NGROUPS_MAX;
#if defined(HAVE_SYSCONF) && defined(_SC_NGROUPS_MAX)
	ngroups = MAX(NGROUPS_MAX, sysconf(_SC_NGROUPS_MAX));
#endif

	groups_bygid = xcalloc(ngroups, sizeof(*groups_bygid));
	groups_byname = xcalloc(ngroups, sizeof(*groups_byname));

	if (getgrouplist(user, base, groups_bygid, &ngroups) == -1)
		logit("getgrouplist: groups list too small");
	for (i = 0, j = 0; i < ngroups; i++)
		if ((gr = getgrgid(groups_bygid[i])) != NULL)
			groups_byname[j++] = xstrdup(gr->gr_name);
	xfree(groups_bygid);
	return (ngroups = j);
}

/*
 * Return 1 if one of user's groups is contained in groups.
 * Return 0 otherwise.  Use match_pattern() for string comparison.
 */
int
ga_match(char * const *groups, int n)
{
	int i, j;

	for (i = 0; i < ngroups; i++)
		for (j = 0; j < n; j++)
			if (match_pattern(groups_byname[i], groups[j]))
				return 1;
	return 0;
}

/*
 * Return 1 if one of user's groups matches group_pattern list.
 * Return 0 on negated or no match.
 */
int
ga_match_pattern_list(const char *group_pattern)
{
	int i, found = 0;
	size_t len = strlen(group_pattern);

	for (i = 0; i < ngroups; i++) {
		switch (match_pattern_list(groups_byname[i],
		    group_pattern, len, 0)) {
		case -1:
			return 0;	/* Negated match wins */
		case 0:
			continue;
		case 1:
			found = 1;
		}
	}
	return found;
}

/*
 * Free memory allocated for group access list.
 */
void
ga_free(void)
{
	int i;

	if (ngroups > 0) {
		for (i = 0; i < ngroups; i++)
			xfree(groups_byname[i]);
		ngroups = 0;
		xfree(groups_byname);
	}
}