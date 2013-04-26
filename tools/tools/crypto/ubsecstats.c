
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
#include "../../../sys/dev/ubsec/ubsecvar.h"

/*
 * Little program to dump the statistics block for the ubsec driver.
 */
int
main(int argc, char *argv[])
{
	struct ubsec_stats stats;
	size_t slen;

	slen = sizeof (stats);
	if (sysctlbyname("hw.ubsec.stats", &stats, &slen, NULL, NULL) < 0)
		err(1, "kern.ubsec_stats");

	printf("input %llu bytes %u packets\n",
		stats.hst_ibytes, stats.hst_ipackets);
	printf("output %llu bytes %u packets\n",
		stats.hst_obytes, stats.hst_opackets);
	printf("invalid %u badsession %u badflags %u\n",
		stats.hst_invalid, stats.hst_badsession, stats.hst_badflags);
	printf("nodesc %u badalg %u nomem %u queuefull %u\n",
		stats.hst_nodesc, stats.hst_badalg, stats.hst_nomem,
		stats.hst_queuefull);
	printf("dmaerr %u mcrerr %u nodmafree %u\n",
		stats.hst_dmaerr, stats.hst_mcrerr, stats.hst_nodmafree);
	printf("lenmismatch %u skipmisatch %u iovmisalined %u\n",
		stats.hst_lenmismatch, stats.hst_skipmismatch,
		stats.hst_iovmisaligned);
	printf("noirq %u unaligned %u nomap %u noload %u nomcl %u\n",
		stats.hst_noirq, stats.hst_unaligned, stats.hst_nomap,
		stats.hst_noload, stats.hst_nomcl);
	printf("totbatch %u maxbatch %u\n",
		stats.hst_totbatch, stats.hst_maxbatch);
	printf("maxqueue %u maxqchip %u mcr1full %u\n",
		stats.hst_maxqueue, stats.hst_maxqchip, stats.hst_mcr1full);
	printf("rng %u modexp %u moexpcrt %u\n",
		stats.hst_rng, stats.hst_modexp, stats.hst_modexpcrt);
	return 0;
}