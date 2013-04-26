
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

#include "namespace.h"
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"

#include "ldpart.h"
#include "setlocale.h"

static int split_lines(char *, const char *);

int
__part_load_locale(const char *name,
		int *using_locale,
		char **locale_buf,
		const char *category_filename,
		int locale_buf_size_max,
		int locale_buf_size_min,
		const char **dst_localebuf)
{
	int		saverr, fd, i, num_lines;
	char		*lbuf, *p;
	const char	*plim;
	char		filename[PATH_MAX];
	struct stat	st;
	size_t		namesize, bufsize;

	/* 'name' must be already checked. */
	if (strcmp(name, "C") == 0 || strcmp(name, "POSIX") == 0) {
		*using_locale = 0;
		return (_LDP_CACHE);
	}

	/*
	 * If the locale name is the same as our cache, use the cache.
	 */
	if (*locale_buf != NULL && strcmp(name, *locale_buf) == 0) {
		*using_locale = 1;
		return (_LDP_CACHE);
	}

	/*
	 * Slurp the locale file into the cache.
	 */
	namesize = strlen(name) + 1;

	/* 'PathLocale' must be already set & checked. */

	/* Range checking not needed, 'name' size is limited */
	strcpy(filename, _PathLocale);
	strcat(filename, "/");
	strcat(filename, name);
	strcat(filename, "/");
	strcat(filename, category_filename);
	if ((fd = _open(filename, O_RDONLY | O_CLOEXEC)) < 0)
		return (_LDP_ERROR);
	if (_fstat(fd, &st) != 0)
		goto bad_locale;
	if (st.st_size <= 0) {
		errno = EFTYPE;
		goto bad_locale;
	}
	bufsize = namesize + st.st_size;
	if ((lbuf = malloc(bufsize)) == NULL) {
		errno = ENOMEM;
		goto bad_locale;
	}
	(void)strcpy(lbuf, name);
	p = lbuf + namesize;
	plim = p + st.st_size;
	if (_read(fd, p, (size_t) st.st_size) != st.st_size)
		goto bad_lbuf;
	/*
	 * Parse the locale file into localebuf.
	 */
	if (plim[-1] != '\n') {
		errno = EFTYPE;
		goto bad_lbuf;
	}
	num_lines = split_lines(p, plim);
	if (num_lines >= locale_buf_size_max)
		num_lines = locale_buf_size_max;
	else if (num_lines >= locale_buf_size_min)
		num_lines = locale_buf_size_min;
	else {
		errno = EFTYPE;
		goto bad_lbuf;
	}
	(void)_close(fd);
	/*
	 * Record the successful parse in the cache.
	 */
	if (*locale_buf != NULL)
		free(*locale_buf);
	*locale_buf = lbuf;
	for (p = *locale_buf, i = 0; i < num_lines; i++)
		dst_localebuf[i] = (p += strlen(p) + 1);
	for (i = num_lines; i < locale_buf_size_max; i++)
		dst_localebuf[i] = NULL;
	*using_locale = 1;

	return (_LDP_LOADED);

bad_lbuf:
	saverr = errno;
	free(lbuf);
	errno = saverr;
bad_locale:
	saverr = errno;
	(void)_close(fd);
	errno = saverr;

	return (_LDP_ERROR);
}

static int
split_lines(char *p, const char *plim)
{
	int i;

	i = 0;
	while (p < plim) {
		if (*p == '\n') {
			*p = '\0';
			i++;
		}
		p++;
	}
	return (i);
}