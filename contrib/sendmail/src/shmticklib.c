
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

#include <sm/gen.h>
SM_RCSID("@(#)$Id: shmticklib.c,v 8.14 2001/09/11 04:05:16 gshapiro Exp $")

#if _FFR_SHM_STATUS
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>

# include "statusd_shm.h"

/*
**  SHMTICK -- increment a shared memory variable
**
**	Parameters:
**		inc_me -- identity of shared memory segment
**		what -- which variable to increment
**
**	Returns:
**		none
*/

void
shmtick(inc_me, what)
	int inc_me;
	int what;
{
	static int shmid = -1;
	static STATUSD_SHM *sp = (STATUSD_SHM *)-1;
	static unsigned int cookie = 0;

	if (shmid < 0)
	{
		int size = sizeof(STATUSD_SHM);

		shmid = shmget(STATUSD_SHM_KEY, size, 0);
		if (shmid < 0)
			return;
	}
	if ((unsigned long *) sp == (unsigned long *)-1)
	{
		sp = (STATUSD_SHM *) shmat(shmid, NULL, 0);
		if ((unsigned long *) sp == (unsigned long *) -1)
			return;
	}
	if (sp->magic != STATUSD_MAGIC)
	{
		/*
		**  possible race condition, wait for
		**  statusd to initialize.
		*/

		return;
	}
	if (what >= STATUSD_LONGS)
		what = STATUSD_LONGS - 1;
	if (inc_me >= STATUSD_LONGS)
		inc_me = STATUSD_LONGS - 1;

	if (sp->ul[STATUSD_COOKIE] != cookie)
	{
		cookie = sp->ul[STATUSD_COOKIE];
		++(sp->ul[inc_me]);
	}
	++(sp->ul[what]);
}
#endif /* _FFR_SHM_STATUS */