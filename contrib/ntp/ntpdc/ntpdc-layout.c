
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
/*
 * ntpdc-layout - print layout of NTP mode 7 request/response packets
 */
#include <stdio.h>
#include <stddef.h>

#include "ntpdc.h"
#include "ntp_stdlib.h"

#if defined(IMPL_XNTPD_OLD) && IMPL_XNTPD != 3
#error Unexpected IMPL_XNTPD
#endif

int
main(void)
{
#include "nl.c"

  return (EXIT_SUCCESS);
}