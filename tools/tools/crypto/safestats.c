
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
#include "../../../sys/dev/safe/safevar.h"

/*
 * Little program to dump the statistics block for the safe driver.
 */
int
main(int argc, char *argv[])
{
	struct safe_stats stats;
	size_t slen;

	slen = sizeof (stats);
	if (sysctlbyname("hw.safe.stats", &stats, &slen, NULL, NULL) < 0)
		err(1, "hw.safe.stats");

	printf("input %llu bytes %u packets\n",
		stats.st_ibytes, stats.st_ipackets);
	printf("output %llu bytes %u packets\n",
		stats.st_obytes, stats.st_opackets);
	printf("invalid %u badsession %u badflags %u\n",
		stats.st_invalid, stats.st_badsession, stats.st_badflags);
	printf("nodesc %u badalg %u ringfull %u\n",
		stats.st_nodesc, stats.st_badalg, stats.st_ringfull);
	printf("peoperr %u dmaerr %u bypasstoobig %u\n",
		stats.st_peoperr, stats.st_dmaerr, stats.st_bypasstoobig);
	printf("skipmismatch %u lenmismatch %u coffmisaligned %u cofftoobig %u\n",
		stats.st_skipmismatch, stats.st_lenmismatch,
		stats.st_coffmisaligned, stats.st_cofftoobig);
	printf("iovmisaligned %u iovnotuniform %u noicvcopy %u\n",
		stats.st_iovmisaligned, stats.st_iovnotuniform,
		stats.st_noicvcopy);
	printf("unaligned %u notuniform %u nomap %u noload %u\n",
		stats.st_unaligned, stats.st_notuniform, stats.st_nomap,
		stats.st_noload);
	printf("nomcl %u mbuf %u maxqchip %u\n",
		stats.st_nomcl, stats.st_nombuf, stats.st_maxqchip);
	printf("rng %u rngalarm %u\n",
		stats.st_rng, stats.st_rngalarm);
	return 0;
}