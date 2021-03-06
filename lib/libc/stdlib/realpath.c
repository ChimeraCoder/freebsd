
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
static char sccsid[] = "@(#)realpath.c	8.1 (Berkeley) 2/16/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/param.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"

/*
 * Find the real name of path, by removing all ".", ".." and symlink
 * components.  Returns (resolved) on success, or (NULL) on failure,
 * in which case the path which caused trouble is left in (resolved).
 */
char *
realpath(const char * __restrict path, char * __restrict resolved)
{
	struct stat sb;
	char *p, *q, *s;
	size_t left_len, resolved_len;
	unsigned symlinks;
	int m, slen;
	char left[PATH_MAX], next_token[PATH_MAX], symlink[PATH_MAX];

	if (path == NULL) {
		errno = EINVAL;
		return (NULL);
	}
	if (path[0] == '\0') {
		errno = ENOENT;
		return (NULL);
	}
	if (resolved == NULL) {
		resolved = malloc(PATH_MAX);
		if (resolved == NULL)
			return (NULL);
		m = 1;
	} else
		m = 0;
	symlinks = 0;
	if (path[0] == '/') {
		resolved[0] = '/';
		resolved[1] = '\0';
		if (path[1] == '\0')
			return (resolved);
		resolved_len = 1;
		left_len = strlcpy(left, path + 1, sizeof(left));
	} else {
		if (getcwd(resolved, PATH_MAX) == NULL) {
			if (m)
				free(resolved);
			else {
				resolved[0] = '.';
				resolved[1] = '\0';
			}
			return (NULL);
		}
		resolved_len = strlen(resolved);
		left_len = strlcpy(left, path, sizeof(left));
	}
	if (left_len >= sizeof(left) || resolved_len >= PATH_MAX) {
		if (m)
			free(resolved);
		errno = ENAMETOOLONG;
		return (NULL);
	}

	/*
	 * Iterate over path components in `left'.
	 */
	while (left_len != 0) {
		/*
		 * Extract the next path component and adjust `left'
		 * and its length.
		 */
		p = strchr(left, '/');
		s = p ? p : left + left_len;
		if (s - left >= sizeof(next_token)) {
			if (m)
				free(resolved);
			errno = ENAMETOOLONG;
			return (NULL);
		}
		memcpy(next_token, left, s - left);
		next_token[s - left] = '\0';
		left_len -= s - left;
		if (p != NULL)
			memmove(left, s + 1, left_len + 1);
		if (resolved[resolved_len - 1] != '/') {
			if (resolved_len + 1 >= PATH_MAX) {
				if (m)
					free(resolved);
				errno = ENAMETOOLONG;
				return (NULL);
			}
			resolved[resolved_len++] = '/';
			resolved[resolved_len] = '\0';
		}
		if (next_token[0] == '\0') {
			/*
			 * Handle consequential slashes.  The path
			 * before slash shall point to a directory.
			 *
			 * Only the trailing slashes are not covered
			 * by other checks in the loop, but we verify
			 * the prefix for any (rare) "//" or "/\0"
			 * occurrence to not implement lookahead.
			 */
			if (lstat(resolved, &sb) != 0) {
				if (m)
					free(resolved);
				return (NULL);
			}
			if (!S_ISDIR(sb.st_mode)) {
				if (m)
					free(resolved);
				errno = ENOTDIR;
				return (NULL);
			}
			continue;
		}
		else if (strcmp(next_token, ".") == 0)
			continue;
		else if (strcmp(next_token, "..") == 0) {
			/*
			 * Strip the last path component except when we have
			 * single "/"
			 */
			if (resolved_len > 1) {
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}
			continue;
		}

		/*
		 * Append the next path component and lstat() it.
		 */
		resolved_len = strlcat(resolved, next_token, PATH_MAX);
		if (resolved_len >= PATH_MAX) {
			if (m)
				free(resolved);
			errno = ENAMETOOLONG;
			return (NULL);
		}
		if (lstat(resolved, &sb) != 0) {
			if (m)
				free(resolved);
			return (NULL);
		}
		if (S_ISLNK(sb.st_mode)) {
			if (symlinks++ > MAXSYMLINKS) {
				if (m)
					free(resolved);
				errno = ELOOP;
				return (NULL);
			}
			slen = readlink(resolved, symlink, sizeof(symlink) - 1);
			if (slen < 0) {
				if (m)
					free(resolved);
				return (NULL);
			}
			symlink[slen] = '\0';
			if (symlink[0] == '/') {
				resolved[1] = 0;
				resolved_len = 1;
			} else if (resolved_len > 1) {
				/* Strip the last path component. */
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}

			/*
			 * If there are any path components left, then
			 * append them to symlink. The result is placed
			 * in `left'.
			 */
			if (p != NULL) {
				if (symlink[slen - 1] != '/') {
					if (slen + 1 >= sizeof(symlink)) {
						if (m)
							free(resolved);
						errno = ENAMETOOLONG;
						return (NULL);
					}
					symlink[slen] = '/';
					symlink[slen + 1] = 0;
				}
				left_len = strlcat(symlink, left,
				    sizeof(symlink));
				if (left_len >= sizeof(left)) {
					if (m)
						free(resolved);
					errno = ENAMETOOLONG;
					return (NULL);
				}
			}
			left_len = strlcpy(left, symlink, sizeof(left));
		}
	}

	/*
	 * Remove trailing slash except when the resolved pathname
	 * is a single "/".
	 */
	if (resolved_len > 1 && resolved[resolved_len - 1] == '/')
		resolved[resolved_len - 1] = '\0';
	return (resolved);
}