
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
/*
 * Public domain stdio wrapper for libz, written by Johan Danielsson.
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: head/lib/libz/zopen.c 84228 2001-09-30 22:39:00Z dillon $");

#include <stdio.h>
#include <zlib.h>

FILE *zopen(const char *fname, const char *mode);

/* convert arguments */
static int
xgzread(void *cookie, char *data, int size)
{
    return gzread(cookie, data, size);
}

static int
xgzwrite(void *cookie, const char *data, int size)
{
    return gzwrite(cookie, (void*)data, size);
}

static int
xgzclose(void *cookie)
{
    return gzclose(cookie);
}

FILE *
zopen(const char *fname, const char *mode)
{
    gzFile gz = gzopen(fname, mode);
    if(gz == NULL)
	return NULL;

    if(*mode == 'r')
	return (funopen(gz, xgzread, NULL, NULL, xgzclose));
    else
	return (funopen(gz, NULL, xgzwrite, NULL, xgzclose));
}