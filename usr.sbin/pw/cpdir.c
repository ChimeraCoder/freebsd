
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

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>

#include "pw.h"
#include "pwupd.h"

void
copymkdir(char const * dir, char const * skel, mode_t mode, uid_t uid, gid_t gid)
{
	char            src[MAXPATHLEN];
	char            dst[MAXPATHLEN];
	char            lnk[MAXPATHLEN];
	int             len;

	if (mkdir(dir, mode) != 0 && errno != EEXIST) {
		warn("mkdir(%s)", dir);
	} else {
		int             infd, outfd;
		struct stat     st;

		static char     counter = 0;
		static char    *copybuf = NULL;

		++counter;
		chown(dir, uid, gid);
		if (skel != NULL && *skel != '\0') {
			DIR            *d = opendir(skel);

			if (d != NULL) {
				struct dirent  *e;

				while ((e = readdir(d)) != NULL) {
					char           *p = e->d_name;

					if (snprintf(src, sizeof(src), "%s/%s", skel, p) >= (int)sizeof(src))
						warn("warning: pathname too long '%s/%s' (skel not copied)", skel, p);
					else if (lstat(src, &st) == 0) {
						if (strncmp(p, "dot.", 4) == 0)	/* Conversion */
							p += 3;
						if (snprintf(dst, sizeof(dst), "%s/%s", dir, p) >= (int)sizeof(dst))
							warn("warning: path too long '%s/%s' (skel file skipped)", dir, p);
						else {
						    if (S_ISDIR(st.st_mode)) {	/* Recurse for this */
							if (strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0)
								copymkdir(dst, src, st.st_mode & _DEF_DIRMODE, uid, gid);
								chflags(dst, st.st_flags);	/* propagate flags */
						    } else if (S_ISLNK(st.st_mode) && (len = readlink(src, lnk, sizeof(lnk) - 1)) != -1) {
							lnk[len] = '\0';
							symlink(lnk, dst);
							lchown(dst, uid, gid);
							/*
							 * Note: don't propagate special attributes
							 * but do propagate file flags
							 */
						    } else if (S_ISREG(st.st_mode) && (outfd = open(dst, O_RDWR | O_CREAT | O_EXCL, st.st_mode)) != -1) {
							if ((infd = open(src, O_RDONLY)) == -1) {
								close(outfd);
								remove(dst);
							} else {
								int             b;

								/*
								 * Allocate our copy buffer if we need to
								 */
								if (copybuf == NULL)
									copybuf = malloc(4096);
								while ((b = read(infd, copybuf, 4096)) > 0)
									write(outfd, copybuf, b);
								close(infd);
								/*
								 * Propagate special filesystem flags
								 */
								fchown(outfd, uid, gid);
								fchflags(outfd, st.st_flags);
								close(outfd);
								chown(dst, uid, gid);
							}
						    }
						}
					}
				}
				closedir(d);
			}
		}
		if (--counter == 0 && copybuf != NULL) {
			free(copybuf);
			copybuf = NULL;
		}
	}
}