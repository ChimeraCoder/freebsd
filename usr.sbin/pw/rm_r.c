
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
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <dirent.h>

#include "pwupd.h"

void
rm_r(char const * dir, uid_t uid)
{
	DIR            *d = opendir(dir);

	if (d != NULL) {
		struct dirent  *e;
		struct stat     st;
		char            file[MAXPATHLEN];

		while ((e = readdir(d)) != NULL) {
			if (strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0) {
				snprintf(file, sizeof(file), "%s/%s", dir, e->d_name);
				if (lstat(file, &st) == 0) {	/* Need symlinks, not
								 * linked file */
					if (S_ISDIR(st.st_mode))	/* Directory - recurse */
						rm_r(file, uid);
					else {
						if (S_ISLNK(st.st_mode) || st.st_uid == uid)
							remove(file);
					}
				}
			}
		}
		closedir(d);
		if (lstat(dir, &st) == 0) {
			if (S_ISLNK(st.st_mode))
				remove(dir);
			else if (st.st_uid == uid)
				rmdir(dir);
		}
	}
}