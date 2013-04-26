
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

/* $Id: os.c,v 1.3 2009/06/11 23:47:55 tbox Exp $ */

/*! \file */

#include <config.h>

#include <confgen/os.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

int
set_user(FILE *fd, const char *user) {
	struct passwd *pw;

	pw = getpwnam(user);
	if (pw == NULL) {
		errno = EINVAL;
		return (-1);
	}
	return (fchown(fileno(fd), pw->pw_uid, -1));
}