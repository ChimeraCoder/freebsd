
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
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>

#include "support.h"

char *
concat(const char *s1, const char *s2, const char *s3)
{
	int len = 1;
	char *s;
	if (s1)
		len += strlen(s1);
	if (s2)
		len += strlen(s2);
	if (s3)
		len += strlen(s3);
	s = xmalloc(len);
	s[0] = '\0';
	if (s1)
		strcat(s, s1);
	if (s2)
		strcat(s, s2);
	if (s3)
		strcat(s, s3);
	return s;
}

void *
xmalloc(size_t n)
{
	char *p = malloc(n);

	if (p == NULL)
		errx(1, "Could not allocate memory");

	return p;
}

void *
xrealloc(void *p, size_t n)
{
	p = realloc(p, n);

	if (p == NULL)
		errx(1, "Could not allocate memory");

	return p;
}