
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
 * trylook.c - test program for lookup.c
 *
 * $FreeBSD$
 */
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>

#include "report.h"
#include "lookup.h"

extern char *ether_ntoa();
extern char *inet_ntoa();

int debug = 0;
char *progname;

void
main(argc, argv)
	int argc;
	char **argv;
{
	int i;
	struct in_addr in;
	char *a;
	u_char *hwa;

	progname = argv[0];			/* for report */

	for (i = 1; i < argc; i++) {

		/* Host name */
		printf("%s:", argv[i]);

		/* IP addr */
		if (lookup_ipa(argv[i], &in.s_addr))
			a = "?";
		else
			a = inet_ntoa(in);
		printf(" ipa=%s", a);

		/* Ether addr */
		printf(" hwa=");
		hwa = lookup_hwa(argv[i], 1);
		if (!hwa)
			printf("?\n");
		else {
			int i;
			for (i = 0; i < 6; i++)
				printf(":%x", hwa[i] & 0xFF);
			putchar('\n');
		}

	}
	exit(0);
}