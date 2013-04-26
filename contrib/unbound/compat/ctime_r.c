
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
/* taken from ldns 1.6.1 */#include "config.h"
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include "util/locks.h"

/** the lock for ctime buffer */
static lock_basic_t ctime_lock;
/** has it been inited */
static int ctime_r_init = 0;

/** cleanup ctime_r on exit */
static void
ctime_r_cleanup(void)
{
	if(ctime_r_init) {
		ctime_r_init = 0;
		lock_basic_destroy(&ctime_lock);
	}
}

char *ctime_r(const time_t *timep, char *buf)
{
	char* result;
	if(!ctime_r_init) {
		/* still small race where this init can be done twice,
		 * which is mostly harmless */
		ctime_r_init = 1;
		lock_basic_init(&ctime_lock);
		atexit(&ctime_r_cleanup);
	}
	lock_basic_lock(&ctime_lock);
	result = ctime(timep);
	if(buf && result) {
		if(strlen(result) > 10 && result[7]==' ' && result[8]=='0')
			result[8]=' '; /* fix error in windows ctime */
		strcpy(buf, result);
	}
	lock_basic_unlock(&ctime_lock);
	return result;
}