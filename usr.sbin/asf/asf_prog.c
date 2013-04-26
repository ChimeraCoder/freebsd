
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
#include <err.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "asf.h"

/*
 * Get the linker file list from kldstat(8) output.
 * The "run" flag tells if kldstat(8) should run now.
 * Of course, kldstat(1) can run in a live system only, but its output
 * can be saved beforehand and fed to this function later via stdin.
 */
void
asf_prog(int run)
{
	char	buf[LINE_MAX];
	char   *token[MAXTOKEN];
	char   *endp;
	FILE   *kldstat;
	caddr_t	base;
	int	tokens;

	if (run) {
		if ((kldstat = popen("kldstat", "r")) == NULL)
			err(2, "can't start kldstat");
	} else
		kldstat = stdin;

	while (fgets(buf, sizeof(buf), kldstat)) {
		/* Skip header line and main kernel file */
		if (buf[0] == 'I' || strstr(buf, KERNFILE))
			continue;
		tokens = tokenize(buf, token, MAXTOKEN);
		if (tokens < 4)
			continue;
		base = (caddr_t)(uintptr_t)strtoumax(token[2], &endp, 16);
		if (endp == NULL || *endp != '\0')
			continue;
		kfile_add(token[4], base);
	}
}