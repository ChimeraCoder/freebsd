
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getlogin.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "namespace.h"
#include <pthread.h>
#include "un-namespace.h"

#include "libc_private.h"

#define	THREAD_LOCK()	if (__isthreaded) _pthread_mutex_lock(&logname_mutex)
#define	THREAD_UNLOCK()	if (__isthreaded) _pthread_mutex_unlock(&logname_mutex)

extern int		_getlogin(char *, int);

int			_logname_valid;		/* known to setlogin() */
static pthread_mutex_t	logname_mutex = PTHREAD_MUTEX_INITIALIZER;

static char *
getlogin_basic(int *status)
{
	static char logname[MAXLOGNAME];

	if (_logname_valid == 0) {
		if (_getlogin(logname, sizeof(logname)) < 0) {
			*status = errno;
			return (NULL);
		}
		_logname_valid = 1;
	}
	*status = 0;
	return (*logname ? logname : NULL);
}

char *
getlogin(void)
{
	char	*result;
	int	status;

	THREAD_LOCK();
	result = getlogin_basic(&status);
	THREAD_UNLOCK();
	return (result);
}

int
getlogin_r(char *logname, int namelen)
{
	char	*result;
	int	len;
	int	status;
	
	THREAD_LOCK();
	result = getlogin_basic(&status);
	if (status == 0) {
		if ((len = strlen(result) + 1) > namelen)
			status = ERANGE;
		else
			strncpy(logname, result, len);
	}
	THREAD_UNLOCK();
	return (status);
}