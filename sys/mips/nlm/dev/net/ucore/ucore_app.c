
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

#include "ucore.h"

int main(void)
{
#if 0
	volatile unsigned int *pkt = 
	    (volatile unsigned int *) (PACKET_MEMORY + PACKET_DATA_OFFSET);
	int intf, hw_parser_error, context;
#endif
	unsigned int pktrdy;
	int num_cachelines = 1518 / 64 ; /* pktsize / L3 cacheline size */


	/* Spray packets to using distribution vector */
	while (1) {
		pktrdy = nlm_read_ucore_rxpktrdy();
#if 0
		intf = pktrdy & 0x1f;
		context = (pktrdy >> 13) & 0x3ff;
		hw_parser_error = (pktrdy >> 23) & 0x1;
#endif
		nlm_ucore_setup_poepktdistr(FWD_DIST_VEC, 0, 0, 0, 0);
		nlm_ucore_pkt_done(num_cachelines, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0);
	}

	return (0);
}