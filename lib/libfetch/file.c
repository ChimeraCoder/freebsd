
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

#include <sys/param.h>
#include <sys/stat.h>

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "fetch.h"
#include "common.h"

FILE *
fetchXGetFile(struct url *u, struct url_stat *us, const char *flags)
{
	FILE *f;

	if (us && fetchStatFile(u, us, flags) == -1)
		return (NULL);

	f = fopen(u->doc, "r");

	if (f == NULL) {
		fetch_syserr();
		return (NULL);
	}

	if (u->offset && fseeko(f, u->offset, SEEK_SET) == -1) {
		fclose(f);
		fetch_syserr();
		return (NULL);
	}

	fcntl(fileno(f), F_SETFD, FD_CLOEXEC);
	return (f);
}

FILE *
fetchGetFile(struct url *u, const char *flags)
{
	return (fetchXGetFile(u, NULL, flags));
}

FILE *
fetchPutFile(struct url *u, const char *flags)
{
	FILE *f;

	if (CHECK_FLAG('a'))
		f = fopen(u->doc, "a");
	else
		f = fopen(u->doc, "w+");

	if (f == NULL) {
		fetch_syserr();
		return (NULL);
	}

	if (u->offset && fseeko(f, u->offset, SEEK_SET) == -1) {
		fclose(f);
		fetch_syserr();
		return (NULL);
	}

	fcntl(fileno(f), F_SETFD, FD_CLOEXEC);
	return (f);
}

static int
fetch_stat_file(const char *fn, struct url_stat *us)
{
	struct stat sb;

	us->size = -1;
	us->atime = us->mtime = 0;
	if (stat(fn, &sb) == -1) {
		fetch_syserr();
		return (-1);
	}
	us->size = sb.st_size;
	us->atime = sb.st_atime;
	us->mtime = sb.st_mtime;
	return (0);
}

int
fetchStatFile(struct url *u, struct url_stat *us, const char *flags __unused)
{
	return (fetch_stat_file(u->doc, us));
}

struct url_ent *
fetchListFile(struct url *u, const char *flags __unused)
{
	struct dirent *de;
	struct url_stat us;
	struct url_ent *ue;
	int size, len;
	char fn[PATH_MAX], *p;
	DIR *dir;
	int l;

	if ((dir = opendir(u->doc)) == NULL) {
		fetch_syserr();
		return (NULL);
	}

	ue = NULL;
	strncpy(fn, u->doc, sizeof(fn) - 2);
	fn[sizeof(fn) - 2] = 0;
	strcat(fn, "/");
	p = strchr(fn, 0);
	l = sizeof(fn) - strlen(fn) - 1;

	while ((de = readdir(dir)) != NULL) {
		strncpy(p, de->d_name, l - 1);
		p[l - 1] = 0;
		if (fetch_stat_file(fn, &us) == -1)
			/* should I return a partial result, or abort? */
			break;
		fetch_add_entry(&ue, &size, &len, de->d_name, &us);
	}

	return (ue);
}