
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

#include <sys/types.h>
#include <sys/endian.h>

#include "tcpp.h"

void
tcpp_header_encode(struct tcpp_header *thp)
{

	thp->th_magic = htobe32(thp->th_magic);
	thp->th_len = htobe64(thp->th_len);
}

void
tcpp_header_decode(struct tcpp_header *thp)
{

	thp->th_magic = be32toh(thp->th_magic);
	thp->th_len = be64toh(thp->th_len);
}