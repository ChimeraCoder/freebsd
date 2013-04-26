
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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rtld.h"
#include "rtld_printf.h"

void *
xcalloc(size_t number, size_t size)
{
	void *p;

	p = calloc(number, size);
	if (p == NULL) {
		rtld_fdputstr(STDERR_FILENO, "Out of memory\n");
		_exit(1);
	}
	return (p);
}

void *
xmalloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL) {
	rtld_fdputstr(STDERR_FILENO, "Out of memory\n");
	_exit(1);
    }
    return p;
}

char *
xstrdup(const char *str)
{
	char *copy;
	size_t len;

	len = strlen(str) + 1;
	copy = xmalloc(len);
	memcpy(copy, str, len);
	return (copy);
}