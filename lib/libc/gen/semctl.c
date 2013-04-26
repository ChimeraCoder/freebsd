
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

#define _WANT_SEMUN_OLD

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdarg.h>
#include <stdlib.h>

int	__semctl(int semid, int semnum, int cmd, union semun *arg);
int	freebsd7___semctl(int semid, int semnum, int cmd, union semun_old *arg);

int
semctl(int semid, int semnum, int cmd, ...)
{
	va_list ap;
	union semun semun;
	union semun *semun_ptr;

	va_start(ap, cmd);
	if (cmd == IPC_SET || cmd == IPC_STAT || cmd == GETALL
	    || cmd == SETVAL || cmd == SETALL) {
		semun = va_arg(ap, union semun);
		semun_ptr = &semun;
	} else {
		semun_ptr = NULL;
	}
	va_end(ap);

	return (__semctl(semid, semnum, cmd, semun_ptr));
}

int
freebsd7_semctl(int semid, int semnum, int cmd, ...)
{
	va_list ap;
	union semun_old semun;
	union semun_old *semun_ptr;

	va_start(ap, cmd);
	if (cmd == IPC_SET || cmd == IPC_STAT || cmd == GETALL
	    || cmd == SETVAL || cmd == SETALL) {
		semun = va_arg(ap, union semun_old);
		semun_ptr = &semun;
	} else {
		semun_ptr = NULL;
	}
	va_end(ap);

	return (freebsd7___semctl(semid, semnum, cmd, semun_ptr));
}

__sym_compat(semctl, freebsd7_semctl, FBSD_1.0);