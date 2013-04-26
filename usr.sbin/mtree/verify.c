
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
static char sccsid[] = "@(#)verify.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fts.h>
#include <fnmatch.h>
#include <stdio.h>
#include <unistd.h>
#include "mtree.h"
#include "extern.h"

static NODE *root;
static char path[MAXPATHLEN];

static void	miss(NODE *, char *);
static int	vwalk(void);

int
mtree_verifyspec(FILE *fi)
{
	int rval;

	root = mtree_readspec(fi);
	rval = vwalk();
	miss(root, path);
	return (rval);
}

static int
nsort(const FTSENT * const *a, const FTSENT * const *b)
{
	return (strcmp((*a)->fts_name, (*b)->fts_name));
}

static int
vwalk(void)
{
	FTS *t;
	FTSENT *p;
	NODE *ep, *level;
	int specdepth, rval;
	char *argv[2];
	char dot[] = ".";

	argv[0] = dot;
	argv[1] = NULL;
	if ((t = fts_open(argv, ftsoptions, nsort)) == NULL)
		err(1, "line %d: fts_open", lineno);
	level = root;
	specdepth = rval = 0;
	while ((p = fts_read(t))) {
		if (check_excludes(p->fts_name, p->fts_path)) {
			fts_set(t, p, FTS_SKIP);
			continue;
		}
		switch(p->fts_info) {
		case FTS_D:
		case FTS_SL:
			break;
		case FTS_DP:
			if (specdepth > p->fts_level) {
				for (level = level->parent; level->prev;
				      level = level->prev);
				--specdepth;
			}
			continue;
		case FTS_DNR:
		case FTS_ERR:
		case FTS_NS:
			warnx("%s: %s", RP(p), strerror(p->fts_errno));
			continue;
		default:
			if (dflag)
				continue;
		}

		if (specdepth != p->fts_level)
			goto extra;
		for (ep = level; ep; ep = ep->next)
			if ((ep->flags & F_MAGIC &&
			    !fnmatch(ep->name, p->fts_name, FNM_PATHNAME)) ||
			    !strcmp(ep->name, p->fts_name)) {
				ep->flags |= F_VISIT;
				if ((ep->flags & F_NOCHANGE) == 0 &&
				    compare(ep->name, ep, p))
					rval = MISMATCHEXIT;
				if (ep->flags & F_IGN)
					(void)fts_set(t, p, FTS_SKIP);
				else if (ep->child && ep->type == F_DIR &&
				    p->fts_info == FTS_D) {
					level = ep->child;
					++specdepth;
				}
				break;
			}

		if (ep)
			continue;
extra:
		if (!eflag) {
			(void)printf("%s extra", RP(p));
			if (rflag) {
				if ((S_ISDIR(p->fts_statp->st_mode)
				    ? rmdir : unlink)(p->fts_accpath)) {
					(void)printf(", not removed: %s",
					    strerror(errno));
				} else
					(void)printf(", removed");
			}
			(void)putchar('\n');
		}
		(void)fts_set(t, p, FTS_SKIP);
	}
	(void)fts_close(t);
	if (sflag)
		warnx("%s checksum: %lu", fullpath, (unsigned long)crc_total);
	return (rval);
}

static void
miss(NODE *p, char *tail)
{
	int create;
	char *tp;
	const char *type, *what;
	int serr;

	for (; p; p = p->next) {
		if (p->flags & F_OPT && !(p->flags & F_VISIT))
			continue;
		if (p->type != F_DIR && (dflag || p->flags & F_VISIT))
			continue;
		(void)strcpy(tail, p->name);
		if (!(p->flags & F_VISIT)) {
			/* Don't print missing message if file exists as a
			   symbolic link and the -q flag is set. */
			struct stat statbuf;

			if (qflag && stat(path, &statbuf) == 0)
				p->flags |= F_VISIT;
			else
				(void)printf("%s missing", path);
		}
		if (p->type != F_DIR && p->type != F_LINK) {
			putchar('\n');
			continue;
		}

		create = 0;
		if (p->type == F_LINK)
			type = "symlink";
		else
			type = "directory";
		if (!(p->flags & F_VISIT) && uflag) {
			if (!(p->flags & (F_UID | F_UNAME)))
				(void)printf(" (%s not created: user not specified)", type);
			else if (!(p->flags & (F_GID | F_GNAME)))
				(void)printf(" (%s not created: group not specified)", type);
			else if (p->type == F_LINK) {
				if (symlink(p->slink, path))
					(void)printf(" (symlink not created: %s)\n",
					    strerror(errno));
				else
					(void)printf(" (created)\n");
				if (lchown(path, p->st_uid, p->st_gid) == -1) {
					serr = errno;
					if (p->st_uid == (uid_t)-1)
						what = "group";
					else if (lchown(path, (uid_t)-1,
					    p->st_gid) == -1)
						what = "user & group";
					else {
						what = "user";
						errno = serr;
					}
					(void)printf("%s: %s not modified: %s"
					    "\n", path, what, strerror(errno));
				}
				continue;
			} else if (!(p->flags & F_MODE))
			    (void)printf(" (directory not created: mode not specified)");
			else if (mkdir(path, S_IRWXU))
				(void)printf(" (directory not created: %s)",
				    strerror(errno));
			else {
				create = 1;
				(void)printf(" (created)");
			}
		}
		if (!(p->flags & F_VISIT))
			(void)putchar('\n');

		for (tp = tail; *tp; ++tp);
		*tp = '/';
		miss(p->child, tp + 1);
		*tp = '\0';

		if (!create)
			continue;
		if (chown(path, p->st_uid, p->st_gid) == -1) {
			serr = errno;
			if (p->st_uid == (uid_t)-1)
				what = "group";
			else if (chown(path, (uid_t)-1, p->st_gid) == -1)
				what = "user & group";
			else {
				what = "user";
				errno = serr;
			}
			(void)printf("%s: %s not modified: %s\n",
			    path, what, strerror(errno));
		}
		if (chmod(path, p->st_mode))
			(void)printf("%s: permissions not set: %s\n",
			    path, strerror(errno));
		if ((p->flags & F_FLAGS) && p->st_flags &&
		    chflags(path, p->st_flags))
			(void)printf("%s: file flags not set: %s\n",
			    path, strerror(errno));
	}
}