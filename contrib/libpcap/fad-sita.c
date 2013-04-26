
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

 /* $Id: fad-sita.c */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "pcap-int.h"

#include "pcap-sita.h"

extern pcap_if_t	*acn_if_list;								/* pcap's list of available interfaces */

int pcap_findalldevs(pcap_if_t **alldevsp, char *errbuf) {

	//printf("pcap_findalldevs()\n");				// fulko

	*alldevsp = 0;												/* initialize the returned variables before we do anything */
	strcpy(errbuf, "");
	if (acn_parse_hosts_file(errbuf))							/* scan the hosts file for potential IOPs */
		{
		//printf("pcap_findalldevs() returning BAD after parsehosts\n");				// fulko
		return -1;
		}
	//printf("pcap_findalldevs() got hostlist now finding devs\n");				// fulko
	if (acn_findalldevs(errbuf))								/* then ask the IOPs for their monitorable devices */
		{
		//printf("pcap_findalldevs() returning BAD after findalldevs\n");				// fulko
		return -1;
		}
	*alldevsp = acn_if_list;
	acn_if_list = 0;											/* then forget our list head, because someone will call pcap_freealldevs() to empty the malloc'ed stuff */
	//printf("pcap_findalldevs() returning ZERO OK\n");				// fulko
	return 0;
}