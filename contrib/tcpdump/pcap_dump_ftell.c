
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
    "@(#) $Header: /tcpdump/master/tcpdump/pcap_dump_ftell.c,v 1.1 2005-06-03 22:08:52 guy Exp $ (LBL)";
#endif

#include <stdio.h>
#include <pcap.h>

#include "pcap-missing.h"

long
pcap_dump_ftell(pcap_dumper_t *p)
{
	return (ftell((FILE *)p));
}