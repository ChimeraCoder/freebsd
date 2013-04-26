
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
#include <stdio.h>
#include <sys/types.h>
#include "../../../sys/dev/hifn/hifn7751var.h"

/*
 * Little program to dump the statistics block for the hifn driver.
 */
int
main(int argc, char *argv[])
{
	struct hifn_stats stats;
	size_t slen;

	slen = sizeof (stats);
	if (sysctlbyname("hw.hifn.stats", &stats, &slen, NULL, NULL) < 0)
		err(1, "kern.hifn.stats");

	printf("input %llu bytes %u packets\n",
		stats.hst_ibytes, stats.hst_ipackets);
	printf("output %llu bytes %u packets\n",
		stats.hst_obytes, stats.hst_opackets);
	printf("invalid %u nomem %u abort %u\n",
		stats.hst_invalid, stats.hst_nomem, stats.hst_abort);
	printf("noirq %u unaligned %u\n",
		stats.hst_noirq, stats.hst_unaligned);
	printf("totbatch %u maxbatch %u\n",
		stats.hst_totbatch, stats.hst_maxbatch);
	printf("nomem: map %u load %u mbuf %u mcl %u cr %u sd %u\n",
		stats.hst_nomem_map, stats.hst_nomem_load,
		stats.hst_nomem_mbuf, stats.hst_nomem_mcl,
		stats.hst_nomem_cr, stats.hst_nomem_sd);
	return 0;
}