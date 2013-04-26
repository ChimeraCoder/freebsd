
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
#include <sys/mac.h>

#include <err.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define	MAXELEMENTS	32

static void
usage(void)
{

	fprintf(stderr, "setpmac [label] [command] [args ...]\n");
	exit (EX_USAGE);
}

int
main(int argc, char *argv[])
{
	const char *shell;
	mac_t label;
	int error;


	if (argc < 3)
		usage();

	error = mac_from_text(&label, argv[1]);
	if (error != 0) {
		perror("mac_from_text");
		return (-1);
	}

	error = mac_set_proc(label);
	if (error != 0) {
		perror(argv[1]);
		return (-1);
	}

	mac_free(label);

	if (argc >= 3) {
		execvp(argv[2], argv + 2);
		err(1, "%s", argv[2]);
	} else {
		if (!(shell = getenv("SHELL")))
			shell = _PATH_BSHELL;
		execlp(shell, shell, "-i", (char *)NULL);
		err(1, "%s", shell);
	}
	/* NOTREACHED */
}