
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

#include <sys/time.h>
#include <sys/socket.h>

#include <stdio.h>
#include <unistd.h>

#include "probe.h"
#include "log.h"
#include "id.h"

struct probe probe;

/* Does select() alter the passed time value ? */
static int
select_changes_time(void)
{
  struct timeval t;

  t.tv_sec = 0;
  t.tv_usec = 100000;
  select(0, NULL, NULL, NULL, &t);
  return t.tv_usec != 100000;
}

#ifndef NOINET6
static int
ipv6_available(void)
{
  int s;

  if ((s = ID0socket(PF_INET6, SOCK_DGRAM, 0)) == -1)
    return 0;

  close(s);
  return 1;
}
#endif

void
probe_Init()
{
  probe.select_changes_time = select_changes_time() ? 1 : 0;
  log_Printf(LogDEBUG, "Select changes time: %s\n",
             probe.select_changes_time ? "yes" : "no");
#ifndef NOINET6
  probe.ipv6_available = ipv6_available() ? 1 : 0;
  log_Printf(LogDEBUG, "IPv6 available: %s\n",
             probe.ipv6_available ? "yes" : "no");
#endif
}