
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

#include <sys/types.h>
#include <sys/acl.h>

#include <err.h>
#include <stdio.h>
#include <string.h>

#include "setfacl.h"

/*
 * read acl text from a file and return the corresponding acl
 */
acl_t
get_acl_from_file(const char *filename)
{
	FILE *file;
	char buf[BUFSIZ];

	if (filename == NULL)
		err(1, "(null) filename in get_acl_from_file()");

	bzero(&buf, sizeof(buf));

	if (strcmp(filename, "-") == 0) {
		if (have_stdin != 0)
			err(1, "cannot specify more than one stdin");
		file = stdin;
		have_stdin = 1;
	} else {
		file = fopen(filename, "r");
		if (file == NULL)
			err(1, "fopen() %s failed", filename);
	}

	fread(buf, sizeof(buf), (size_t)1, file);
	if (ferror(file) != 0) {
		fclose(file);
		err(1, "error reading from %s", filename);
	} else if (feof(file) == 0) {
		fclose(file);
		errx(1, "line too long in %s", filename);
	}

	fclose(file);

	return (acl_from_text(buf));
}