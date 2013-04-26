
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/endian.h>

uint32_t
htonl(uint32_t hl)
{

	return (__htonl(hl));
}

uint16_t
htons(uint16_t hs)
{
	
	return (__htons(hs));
}

uint32_t
ntohl(uint32_t nl)
{

	return (__ntohl(nl));
}

uint16_t
ntohs(uint16_t ns)
{
	
	return (__ntohs(ns));
}