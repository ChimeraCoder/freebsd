
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
#include "diag.h"

#include "ah.h"
#include "ah_diagcodes.h"

#include <getopt.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct	ath_diag atd;
int	s;
const char *progname;

/* XXX this should likely be defined somewhere in the HAL */
/* XXX This is a lot larger than the v14 ROM */
#define	MAX_EEPROM_SIZE		16384

uint16_t eep[MAX_EEPROM_SIZE];

static void
usage()
{
	fprintf(stderr, "	%s [-i ifname] -d <dumpfile>\n", progname);
	exit(-1);
}

#define	NUM_PER_LINE	8

static void
do_eeprom_dump(const char *dumpfile, uint16_t *eebuf, int eelen)
{
	FILE *fp;
	int i;

	fp = fopen(dumpfile, "w");
	if (!fp) {
		err(1, "fopen");
	}

	/* eelen is in bytes; eebuf is in 2 byte words */
	for (i = 0; i < eelen / 2; i++) {
		if (i % NUM_PER_LINE == 0)
			fprintf(fp, "%.4x: ", i);
		fprintf(fp, "%.4x%s", (int32_t)(eebuf[i]), i % NUM_PER_LINE == (NUM_PER_LINE - 1) ? "\n" : " ");
	}
	fprintf(fp, "\n");
	fclose(fp);
}

int
main(int argc, char *argv[])
{
	FILE *fd = NULL;
	const char *ifname;
	int c;
	const char *dumpname = NULL;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(1, "socket");
	ifname = getenv("ATH");
	if (!ifname)
		ifname = ATH_DEFAULT;

	progname = argv[0];
	while ((c = getopt(argc, argv, "d:i:t:")) != -1)
		switch (c) {
		case 'd':
			dumpname = optarg;
			break;
		case 'i':
			ifname = optarg;
			break;
		case 't':
			fd = fopen(optarg, "r");
			if (fd == NULL)
				err(-1, "Cannot open %s", optarg);
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	argc -= optind;
	argv += optind;

	strncpy(atd.ad_name, ifname, sizeof (atd.ad_name));

	/* Read in the entire EEPROM */
	atd.ad_id = HAL_DIAG_EEPROM;
	atd.ad_out_data = (caddr_t) eep;
	atd.ad_out_size = sizeof(eep);
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, "ioctl: %s", atd.ad_name);

	/* Dump file? Then just write to it */
	if (dumpname != NULL) {
		do_eeprom_dump(dumpname, (uint16_t *) &eep, sizeof(eep));
	}
	return 0;
}