
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
static char sccsid[] = "@(#)scandir.c	8.3 (Berkeley) 1/2/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Scan the directory dirname calling select to make a list of selected
 * directory entries then sort using qsort and compare routine dcomp.
 * Returns the number of entries and a pointer to a list of pointers to
 * struct dirent (through namelist). Returns -1 if there were any errors.
 */

#include "namespace.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "un-namespace.h"

static int alphasort_thunk(void *thunk, const void *p1, const void *p2);

/*
 * The DIRSIZ macro is the minimum record length which will hold the directory
 * entry.  This requires the amount of space in struct dirent without the
 * d_name field, plus enough space for the name and a terminating nul byte
 * (dp->d_namlen + 1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp)							\
	((sizeof(struct dirent) - sizeof(dp)->d_name) +			\
	    (((dp)->d_namlen + 1 + 3) &~ 3))

int
scandir(const char *dirname, struct dirent ***namelist,
    int (*select)(const struct dirent *), int (*dcomp)(const struct dirent **,
	const struct dirent **))
{
	struct dirent *d, *p, **names = NULL;
	size_t nitems = 0;
	long arraysz;
	DIR *dirp;

	if ((dirp = opendir(dirname)) == NULL)
		return(-1);

	arraysz = 32;	/* initial estimate of the array size */
	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		goto fail;

	while ((d = readdir(dirp)) != NULL) {
		if (select != NULL && !(*select)(d))
			continue;	/* just selected names */
		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL)
			goto fail;
		p->d_fileno = d->d_fileno;
		p->d_type = d->d_type;
		p->d_reclen = d->d_reclen;
		p->d_namlen = d->d_namlen;
		bcopy(d->d_name, p->d_name, p->d_namlen + 1);
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (nitems >= arraysz) {
			struct dirent **names2;

			names2 = (struct dirent **)realloc((char *)names,
				(arraysz * 2) * sizeof(struct dirent *));
			if (names2 == NULL) {
				free(p);
				goto fail;
			}
			names = names2;
			arraysz *= 2;
		}
		names[nitems++] = p;
	}
	closedir(dirp);
	if (nitems && dcomp != NULL)
		qsort_r(names, nitems, sizeof(struct dirent *),
		    &dcomp, alphasort_thunk);
	*namelist = names;
	return (nitems);

fail:
	while (nitems > 0)
		free(names[--nitems]);
	free(names);
	closedir(dirp);
	return (-1);
}

/*
 * Alphabetic order comparison routine for those who want it.
 * POSIX 2008 requires that alphasort() uses strcoll().
 */
int
alphasort(const struct dirent **d1, const struct dirent **d2)
{

	return (strcoll((*d1)->d_name, (*d2)->d_name));
}

static int
alphasort_thunk(void *thunk, const void *p1, const void *p2)
{
	int (*dc)(const struct dirent **, const struct dirent **);

	dc = *(int (**)(const struct dirent **, const struct dirent **))thunk;
	return (dc((const struct dirent **)p1, (const struct dirent **)p2));
}