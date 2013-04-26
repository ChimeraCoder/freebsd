
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

	fprintf(stderr, "getpmac [-l list,of,labels] [-p pid]\n");
	exit (EX_USAGE);
}

int
main(int argc, char *argv[])
{
	char *labellist, *string;
	mac_t label;
	pid_t pid;
	int ch, error, pid_set;

	pid_set = 0;
	pid = 0;
	labellist = NULL;
	while ((ch = getopt(argc, argv, "l:p:")) != -1) {
		switch (ch) {
		case 'l':
			if (labellist != NULL)
				usage();
			labellist = argv[optind - 1];
			break;
		case 'p':
			if (pid_set)
				usage();
			pid = atoi(argv[optind - 1]);
			pid_set = 1;
			break;
		default:
			usage();
		}

	}

	argc -= optind;
	argv += optind;

	if (argc != 0)
		usage();

	if (labellist != NULL)
		error = mac_prepare(&label, labellist);
	else
		error = mac_prepare_process_label(&label);
	if (error != 0) {
		perror("mac_prepare");
		return (-1);
	}

	if (pid_set) {
		error = mac_get_pid(pid, label);
		if (error)
			perror("mac_get_pid");
	} else {
		error = mac_get_proc(label);
		if (error)
			perror("mac_get_proc");
	}
	if (error) {
		mac_free(label);
		exit (-1);
	}
	error = mac_to_text(label, &string);
	if (error != 0) {
		perror("mac_to_text");
		exit(EX_DATAERR);
	}

	if (strlen(string) > 0)
		printf("%s\n", string);
		
	mac_free(label);
	free(string);
	exit(EX_OK);
}