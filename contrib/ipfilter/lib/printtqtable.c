
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

#include <fcntl.h>
#include <sys/ioctl.h>
#include "ipf.h"


void printtqtable(table)
ipftq_t *table;
{
	int i;

	printf("TCP Entries per state\n");
	for (i = 0; i < IPF_TCP_NSTATES; i++)
		printf(" %5d", i);
	printf("\n");

	for (i = 0; i < IPF_TCP_NSTATES; i++)
		printf(" %5d", table[i].ifq_ref - 1);
	printf("\n");
}