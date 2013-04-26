
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
/*	dup2 -- 7th Edition UNIX system call emulation for UNIX System V

	last edit:	11-Feb-1987	D A Gwyn
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include	<errno.h>
#include	<fcntl.h>

extern int	close(), fcntl();

int
dup2( oldfd, newfd )
	int		oldfd;		/* already-open file descriptor */
	int		newfd;		/* desired duplicate descriptor */
{
	register int	ret;		/* for fcntl() return value */
	register int	save;		/* for saving entry errno */

	if ( oldfd == newfd )
		return oldfd;		/* be careful not to close() */

	save = errno;			/* save entry errno */
	(void) close( newfd );		/* in case newfd is open */
	/* (may have just clobbered the original errno value) */

	ret = fcntl( oldfd, F_DUPFD, newfd );	/* dupe it */

	if ( ret >= 0 )
		errno = save;		/* restore entry errno */
	else				/* fcntl() returned error */
		if ( errno == EINVAL )
			errno = EBADF;	/* we think of everything */

	return ret;			/* return file descriptor */
}