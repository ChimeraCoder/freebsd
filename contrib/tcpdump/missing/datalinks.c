
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

#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/tcpdump/missing/datalinks.c,v 1.3 2003-11-16 09:36:47 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <pcap.h>
#include <stdlib.h>
#include <stdio.h>

#include "pcap-missing.h"

/*
 * Stub versions for platforms that don't support them.
 */
int
pcap_list_datalinks(pcap_t *p, int **dlt_buffer)
{
	/*
	 * This platform doesn't support changing the DLT for an
	 * interface.  Return a list of DLTs containing only the
	 * DLT this device supports.
	 */
	*dlt_buffer = (int*)malloc(sizeof(**dlt_buffer));
	if (*dlt_buffer == NULL)
		return (-1);
	**dlt_buffer = pcap_datalink(p);
	return (1);
}