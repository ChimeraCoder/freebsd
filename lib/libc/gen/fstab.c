
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
static char sccsid[] = "@(#)fstab.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <errno.h>
#include <fstab.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vis.h>
#include "un-namespace.h"

static FILE *_fs_fp;
static struct fstab _fs_fstab;
static int LineNo = 0;
static char *path_fstab;
static char fstab_path[PATH_MAX];
static int fsp_set = 0;

static void error(int);
static void fixfsfile(void);
static int fstabscan(void);

void
setfstab(const char *file)
{

	if (file == NULL) {
		path_fstab = _PATH_FSTAB;
	} else {
		strncpy(fstab_path, file, PATH_MAX);
		fstab_path[PATH_MAX - 1] = '\0';
		path_fstab = fstab_path;
	}
	fsp_set = 1;

	return;
}

const char *
getfstab(void)
{

	if (fsp_set)
		return (path_fstab);
	else
		return (_PATH_FSTAB);
}

static void
fixfsfile(void)
{
	static char buf[sizeof(_PATH_DEV) + MNAMELEN];
	struct stat sb;
	struct statfs sf;

	if (_fs_fstab.fs_file != NULL && strcmp(_fs_fstab.fs_file, "/") != 0)
		return;
	if (statfs("/", &sf) != 0)
		return;
	if (sf.f_mntfromname[0] == '/')
		buf[0] = '\0';
	else
		strcpy(buf, _PATH_DEV);
	strcat(buf, sf.f_mntfromname);
	if (stat(buf, &sb) != 0 ||
	    (!S_ISBLK(sb.st_mode) && !S_ISCHR(sb.st_mode)))
		return;
	_fs_fstab.fs_spec = buf;
}

static int
fstabscan(void)
{
	char *cp, *p;
#define	MAXLINELENGTH	1024
	static char line[MAXLINELENGTH];
	char subline[MAXLINELENGTH];
	int typexx;

	for (;;) {

		if (!(p = fgets(line, sizeof(line), _fs_fp)))
			return (0);
/* OLD_STYLE_FSTAB */
		++LineNo;
		if (*line == '#' || *line == '\n')
			continue;
		if (!strpbrk(p, " \t")) {
			_fs_fstab.fs_spec = strsep(&p, ":\n");
			_fs_fstab.fs_file = strsep(&p, ":\n");
			fixfsfile();
			_fs_fstab.fs_type = strsep(&p, ":\n");
			if (_fs_fstab.fs_type) {
				if (!strcmp(_fs_fstab.fs_type, FSTAB_XX))
					continue;
				_fs_fstab.fs_mntops = _fs_fstab.fs_type;
				_fs_fstab.fs_vfstype =
				    strcmp(_fs_fstab.fs_type, FSTAB_SW) ?
				    "ufs" : "swap";
				if ((cp = strsep(&p, ":\n")) != NULL) {
					_fs_fstab.fs_freq = atoi(cp);
					if ((cp = strsep(&p, ":\n")) != NULL) {
						_fs_fstab.fs_passno = atoi(cp);
						return (1);
					}
				}
			}
			goto bad;
		}
/* OLD_STYLE_FSTAB */
		while ((cp = strsep(&p, " \t\n")) != NULL && *cp == '\0')
			;
		_fs_fstab.fs_spec = cp;
		if (_fs_fstab.fs_spec == NULL || *_fs_fstab.fs_spec == '#')
			continue;
		if (strunvis(_fs_fstab.fs_spec, _fs_fstab.fs_spec) < 0)
			goto bad;
		while ((cp = strsep(&p, " \t\n")) != NULL && *cp == '\0')
			;
		_fs_fstab.fs_file = cp;
		if (_fs_fstab.fs_file == NULL)
			goto bad;
		if (strunvis(_fs_fstab.fs_file, _fs_fstab.fs_file) < 0)
			goto bad;
		fixfsfile();
		while ((cp = strsep(&p, " \t\n")) != NULL && *cp == '\0')
			;
		_fs_fstab.fs_vfstype = cp;
		while ((cp = strsep(&p, " \t\n")) != NULL && *cp == '\0')
			;
		_fs_fstab.fs_mntops = cp;
		if (_fs_fstab.fs_mntops == NULL)
			goto bad;
		_fs_fstab.fs_freq = 0;
		_fs_fstab.fs_passno = 0;
		while ((cp = strsep(&p, " \t\n")) != NULL && *cp == '\0')
			;
		if (cp != NULL) {
			_fs_fstab.fs_freq = atoi(cp);
			while ((cp = strsep(&p, " \t\n")) != NULL && *cp == '\0')
				;
			if (cp != NULL)
				_fs_fstab.fs_passno = atoi(cp);
		}
		strcpy(subline, _fs_fstab.fs_mntops);
		p = subline;
		for (typexx = 0, cp = strsep(&p, ","); cp;
		     cp = strsep(&p, ",")) {
			if (strlen(cp) != 2)
				continue;
			if (!strcmp(cp, FSTAB_RW)) {
				_fs_fstab.fs_type = FSTAB_RW;
				break;
			}
			if (!strcmp(cp, FSTAB_RQ)) {
				_fs_fstab.fs_type = FSTAB_RQ;
				break;
			}
			if (!strcmp(cp, FSTAB_RO)) {
				_fs_fstab.fs_type = FSTAB_RO;
				break;
			}
			if (!strcmp(cp, FSTAB_SW)) {
				_fs_fstab.fs_type = FSTAB_SW;
				break;
			}
			if (!strcmp(cp, FSTAB_XX)) {
				_fs_fstab.fs_type = FSTAB_XX;
				typexx++;
				break;
			}
		}
		if (typexx)
			continue;
		if (cp != NULL)
			return (1);

bad:		/* no way to distinguish between EOF and syntax error */
		error(EFTYPE);
	}
	/* NOTREACHED */
}

struct fstab *
getfsent(void)
{

	if ((!_fs_fp && !setfsent()) || !fstabscan())
		return (NULL);
	return (&_fs_fstab);
}

struct fstab *
getfsspec(const char *name)
{

	if (setfsent())
		while (fstabscan())
			if (!strcmp(_fs_fstab.fs_spec, name))
				return (&_fs_fstab);
	return (NULL);
}

struct fstab *
getfsfile(const char *name)
{

	if (setfsent())
		while (fstabscan())
			if (!strcmp(_fs_fstab.fs_file, name))
				return (&_fs_fstab);
	return (NULL);
}

int
setfsent(void)
{
	if (_fs_fp) {
		rewind(_fs_fp);
		LineNo = 0;
		return (1);
	}
	if (fsp_set == 0) {
		if (issetugid())
			setfstab(NULL);
		else
			setfstab(getenv("PATH_FSTAB"));
	}
	if ((_fs_fp = fopen(path_fstab, "re")) != NULL) {
		LineNo = 0;
		return (1);
	}
	error(errno);
	return (0);
}

void
endfsent(void)
{

	if (_fs_fp) {
		(void)fclose(_fs_fp);
		_fs_fp = NULL;
	}

	fsp_set = 0;
}

static void
error(int err)
{
	char *p;
	char num[30];

	(void)_write(STDERR_FILENO, "fstab: ", 7);
	(void)_write(STDERR_FILENO, path_fstab, strlen(path_fstab));
	(void)_write(STDERR_FILENO, ":", 1);
	sprintf(num, "%d: ", LineNo);
	(void)_write(STDERR_FILENO, num, strlen(num));
	p = strerror(err);
	(void)_write(STDERR_FILENO, p, strlen(p));
	(void)_write(STDERR_FILENO, "\n", 1);
}