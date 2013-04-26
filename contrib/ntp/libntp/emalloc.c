
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
 * emalloc - return new memory obtained from the system.  Belch if none.
 */#include "ntp_types.h"
#include "ntp_malloc.h"
#include "ntp_syslog.h"
#include "ntp_stdlib.h"

#if defined SYS_WINNT && defined DEBUG
#include <crtdbg.h>
#endif

#if defined SYS_WINNT && defined DEBUG

void *
debug_emalloc(
	u_int size,
	char *filename,
	int line
	)
{
	char *mem;

	if ((mem = (char *)_malloc_dbg(size, _NORMAL_BLOCK, filename, line)) == 0) {
		msyslog(LOG_ERR, "Exiting: No more memory!");
		exit(1);
	}
	return mem;
}

#else

void *
emalloc(
	u_int size
	)
{
	char *mem;

	if ((mem = (char *)malloc(size)) == 0) {
		msyslog(LOG_ERR, "Exiting: No more memory!");
		exit(1);
	}
	return mem;
}


#endif