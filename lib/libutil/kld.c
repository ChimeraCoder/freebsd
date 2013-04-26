
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

#include <sys/param.h>
#include <sys/linker.h>
#include <sys/module.h>

#include <errno.h>
#include <libutil.h>
#include <string.h>

int
kld_isloaded(const char *name)
{
	struct kld_file_stat fstat;
	struct module_stat mstat;
	const char *ko;
	int fid, mid;

	for (fid = kldnext(0); fid > 0; fid = kldnext(fid)) {
		fstat.version = sizeof(fstat);
		if (kldstat(fid, &fstat) != 0)
			continue;
		/* check if the file name matches the supplied name */
		if (strcmp(fstat.name, name) == 0)
			return (1);
		/* strip .ko and try again */
		if ((ko = strstr(fstat.name, ".ko")) != NULL &&
		    strlen(name) == (size_t)(ko - fstat.name) &&
		    strncmp(fstat.name, name, ko - fstat.name) == 0)
			return (1);
		/* look for a matching module within the file */
		for (mid = kldfirstmod(fid); mid > 0; mid = modfnext(mid)) {
			mstat.version = sizeof(mstat);
			if (modstat(mid, &mstat) != 0)
				continue;
			if (strcmp(mstat.name, name) == 0)
				return (1);
		}
	}
	return (0);
}

int
kld_load(const char *name)
{
	if (kldload(name) == -1 && errno != EEXIST)
		return (-1);
	return (0);
}