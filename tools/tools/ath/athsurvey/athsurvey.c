
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
#include "ah_internal.h"

#include <getopt.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

const char *progname;

static void
usage()
{
	fprintf(stderr, "usage: %s [-i ifname]\n", progname);
	exit(-1);
}

static int
get_survey_stats(int s, const char *ifname, HAL_CHANNEL_SURVEY *hs)
{
	uint16_t eedata;
	struct ath_diag atd;

	memset(&atd, '\0', sizeof(atd));

	atd.ad_id = HAL_DIAG_CHANSURVEY | ATH_DIAG_OUT;
	atd.ad_in_size = 0;
	atd.ad_in_data = NULL;
	atd.ad_out_size = sizeof(HAL_CHANNEL_SURVEY);
	atd.ad_out_data = (caddr_t) hs;
	strncpy(atd.ad_name, ifname, sizeof(atd.ad_name));

	if (ioctl(s, SIOCGATHDIAG, &atd) < 0) {
		err(1, "ioctl: %s", atd.ad_name);
		return (0);
	}
	return (1);
}

static void
process_survey_stats(HAL_CHANNEL_SURVEY *hs)
{
	int i;
	float tx = 0.0, rx = 0.0, cc = 0.0, cext = 0.0;
	float max_tx = 0.0, max_rx = 0.0, max_cc = 0.0, max_cext = 0.0;
	uint64_t avg_tx = 0, avg_rx = 0, avg_cc = 0, avg_cext = 0;
	float min_tx = 100.0, min_rx = 100.0, min_cc = 100.0, min_cext = 100.0;
	int n = 0;
	int cycle_count = 0, max_cycle_count = 0;

	/* Calculate a percentage channel busy */
	for (i = 0; i < CHANNEL_SURVEY_SAMPLE_COUNT; i++) {
		/*
		 * Skip samples with no cycle count
		 */
		if (hs->samples[i].cycle_count == 0)
			continue;
		n++;

		/*
		 * Grab cycle count, calculate maximum just for curiousity
		 */
		cycle_count = hs->samples[i].cycle_count;
		if (cycle_count > max_cycle_count)
			max_cycle_count = cycle_count;

		/*
		 * Calculate percentage
		 */
		tx = (float) hs->samples[i].tx_busy * 100.0 /
		    hs->samples[i].cycle_count;
		rx = (float) hs->samples[i].rx_busy * 100.0 /
		    hs->samples[i].cycle_count;
		cc = (float) hs->samples[i].chan_busy * 100.0 /
		    hs->samples[i].cycle_count;
		cext = (float) hs->samples[i].ext_chan_busy * 100.0 /
		    hs->samples[i].cycle_count;

		/*
		 * Update rolling average
		 * XXX to preserve some accuracy, keep two decimal points
		 * using "fixed" point math.
		 */
		avg_tx += (uint64_t) hs->samples[i].tx_busy * 10000 /
		    hs->samples[i].cycle_count;
		avg_rx += (uint64_t) hs->samples[i].rx_busy * 10000 /
		    hs->samples[i].cycle_count;
		avg_cc += (uint64_t) hs->samples[i].chan_busy * 10000 /
		    hs->samples[i].cycle_count;
		avg_cext += (uint64_t) hs->samples[i].ext_chan_busy * 10000 /
		    hs->samples[i].cycle_count;

		/*
		 * Update maximum values
		 */
		if (tx > max_tx)
			max_tx = tx;
		if (rx > max_rx)
			max_rx = rx;
		if (cc > max_cc)
			max_cc = cc;
		if (cext > max_cext)
			max_cext = cext;

		/*
		 * Update minimum values
		 */
		if (tx < min_tx)
			min_tx = tx;
		if (rx < min_rx)
			min_rx = rx;
		if (cc < min_cc)
			min_cc = cc;
		if (cext < min_cext)
			min_cext = cext;
	}

	printf("(%4.1f %4.1f %4.1f %4.1f) ",
	    min_tx, min_rx, min_cc, min_cext);
	printf("(%4.1f %4.1f %4.1f %4.1f) ",
	    n == 0 ? 0.0 : (float) (avg_tx / n) / 100.0,
	    n == 0 ? 0.0 : (float) (avg_rx / n) / 100.0,
	    n == 0 ? 0.0 : (float) (avg_cc / n) / 100.0,
	    n == 0 ? 0.0 : (float) (avg_cext / n) / 100.0);
	printf("(%4.1f %4.1f %4.1f %4.1f)\n",
	    max_tx, max_rx, max_cc, max_cext);
}

int
main(int argc, char *argv[])
{
	FILE *fd = NULL;
	const char *ifname;
	int c, s;
	int l = 0;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(1, "socket");
	ifname = getenv("ATH");
	if (!ifname)
		ifname = ATH_DEFAULT;

	progname = argv[0];
	while ((c = getopt(argc, argv, "i:")) != -1)
		switch (c) {
		case 'i':
			ifname = optarg;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	argc -= optind;
	argv += optind;

	/* Now, loop over and fetch some statistics .. */
	while (1) {
		HAL_CHANNEL_SURVEY hs;
		memset(&hs, '\0', sizeof(hs));
		if (get_survey_stats(s, ifname, &hs) == 0)
			break;
		/* XXX screen height! */
		if (l % 23 == 0) {
			printf("         "
			    "min                   "
			    "avg                   "
			    "max\n");
			printf("  tx%%  rx%%  bc%%  ec%%  ");
			printf("  tx%%  rx%%  bc%%  ec%%  ");
			printf("  tx%%  rx%%  bc%%  ec%%\n");
		}
		process_survey_stats(&hs);
		sleep(1);
		l++;
	}

	return (0);
}