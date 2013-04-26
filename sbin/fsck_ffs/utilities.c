
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
static const char sccsid[] = "@(#)utilities.c	8.6 (Berkeley) 5/19/95";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ffs/fs.h>

#include <err.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <fstab.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "fsck.h"


char *
blockcheck(char *origname)
{
	struct stat stblock;
	char *newname, *cp;
	struct fstab *fsinfo;
	int retried = 0, len;
	static char device[MAXPATHLEN];

	newname = origname;
	if (stat(newname, &stblock) < 0) {
		cp = strrchr(newname, '/');
		if (cp == 0) {
			(void)snprintf(device, sizeof(device), "%s%s",
				_PATH_DEV, newname);
			newname = device;
		}
	}
retry:
	if (stat(newname, &stblock) < 0) {
		printf("Can't stat %s: %s\n", newname, strerror(errno));
		return (origname);
	}
	switch(stblock.st_mode & S_IFMT) {
	case S_IFCHR:
	case S_IFBLK:
		return(newname);
	case S_IFDIR:
		if (retried)
			break;

		len = strlen(origname) - 1;
		if (len > 0 && origname[len] == '/')
			/* remove trailing slash */
			origname[len] = '\0';
		if ((fsinfo = getfsfile(origname)) == NULL) {
			printf(
			    "Can't resolve %s to character special device.\n",
			    origname);
			return (origname);
		}
		newname = fsinfo->fs_spec;
		retried++;
		goto retry;
	}
	/*
	 * Not a block or character device, just return name and
	 * let the user decide whether to use it.
	 */
	return (origname);
}

void
infohandler(int sig __unused)
{
	got_siginfo = 1;
}

void
alarmhandler(int sig __unused)
{
	got_sigalarm = 1;
}