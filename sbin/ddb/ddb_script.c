
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
#include <sys/sysctl.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "ddb.h"

/*
 * These commands manage DDB(4) scripts from user space.  For better or worse,
 * the setting and unsetting of scripts is only poorly represented using
 * sysctl(8), and this interface provides a more user-friendly way to
 * accomplish this management, wrapped around lower-level sysctls.  For
 * completeness, listing of scripts is also included.
 */

#define	SYSCTL_SCRIPT	"debug.ddb.scripting.script"
#define	SYSCTL_SCRIPTS	"debug.ddb.scripting.scripts"
#define	SYSCTL_UNSCRIPT	"debug.ddb.scripting.unscript"

/*
 * Print all scripts (scriptname==NULL) or a specific script.
 */
static void
ddb_list_scripts(const char *scriptname)
{
	char *buffer, *line, *nextline;
	char *line_script, *line_scriptname;
	size_t buflen, len;
	int ret;

repeat:
	if (sysctlbyname(SYSCTL_SCRIPTS, NULL, &buflen, NULL, 0) < 0)
		err(EX_OSERR, "sysctl: %s", SYSCTL_SCRIPTS);
	if (buflen == 0)
		return;
	buffer = malloc(buflen);
	if (buffer == NULL)
		err(EX_OSERR, "malloc");
	bzero(buffer, buflen);
	len = buflen;
	ret = sysctlbyname(SYSCTL_SCRIPTS, buffer, &len, NULL, 0);
	if (ret < 0 && errno != ENOMEM)
		err(EX_OSERR, "sysctl: %s", SYSCTL_SCRIPTS);
	if (ret < 0) {
		free(buffer);
		goto repeat;
	}

	/*
	 * We nul'd the buffer before calling sysctl(), so at worst empty.
	 *
	 * If a specific script hasn't been requested, print it all.
	 */
	if (scriptname == NULL) {
		printf("%s", buffer);
		free(buffer);
		return;
	}

	/*
	 * If a specific script has been requested, we have to parse the
	 * string to find it.
	 */
	nextline = buffer;
	while ((line = strsep(&nextline, "\n")) != NULL) {
		line_script = line;
		line_scriptname = strsep(&line_script, "=");
		if (line_script == NULL)
			continue;
		if (strcmp(scriptname, line_scriptname) != 0)
			continue;
		printf("%s\n", line_script);
		break;
	}
	if (line == NULL) {
		errno = ENOENT;
		err(EX_DATAERR, "%s", scriptname);
	}
	free(buffer);
}

/*
 * "ddb script" can be used to either print or set a script.
 */
void
ddb_script(int argc, char *argv[])
{

	if (argc != 2)
		usage();
	argv++;
	argc--;
	if (strchr(argv[0], '=') != 0) {
		if (sysctlbyname(SYSCTL_SCRIPT, NULL, NULL, argv[0],
		    strlen(argv[0]) + 1) < 0)
			err(EX_OSERR, "sysctl: %s", SYSCTL_SCRIPTS);
	} else
		ddb_list_scripts(argv[0]);
}

void
ddb_scripts(int argc, char *argv[])
{

	if (argc != 1)
		usage();
	ddb_list_scripts(NULL);
}

void
ddb_unscript(int argc, char *argv[])
{
	int ret;

	if (argc != 2)
		usage();
	argv++;
	argc--;
	ret = sysctlbyname(SYSCTL_UNSCRIPT, NULL, NULL, argv[0],
	    strlen(argv[0]) + 1);
	if (ret < 0 && errno == EINVAL) {
		errno = ENOENT;
		err(EX_DATAERR, "sysctl: %s", argv[0]);
	} else if (ret < 0)
		err(EX_OSERR, "sysctl: %s", SYSCTL_UNSCRIPT);
}