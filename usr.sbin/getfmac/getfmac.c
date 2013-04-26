
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

	fprintf(stderr,
	    "getfmac [-h] [-l list,of,labels] [file1] [file2 ...]\n");
	exit (EX_USAGE);
}

int
main(int argc, char *argv[])
{
	char *labellist, *string;
	mac_t label;
	int ch, hflag;
	int error, i;

	labellist = NULL;
	hflag = 0;
	while ((ch = getopt(argc, argv, "hl:")) != -1) {
		switch (ch) {
		case 'h':
			hflag = 1;
			break;
		case 'l':
			if (labellist != NULL)
				usage();
			labellist = argv[optind - 1];
			break;
		default:
			usage();
		}

	}

	for (i = optind; i < argc; i++) {
		if (labellist != NULL)
			error = mac_prepare(&label, labellist);
		else
			error = mac_prepare_file_label(&label);

		if (error != 0) {
			perror("mac_prepare");
			return (-1);
		}

		if (hflag)
			error = mac_get_link(argv[i], label);
		else
			error = mac_get_file(argv[i], label);
		if (error) {
			perror(argv[i]);
			mac_free(label);
			continue;
		}

		error = mac_to_text(label, &string);
		if (error != 0)
			perror("mac_to_text");
		else {
			printf("%s: %s\n", argv[i], string);
			free(string);
		}
		mac_free(label);
	}

	exit(EX_OK);
}