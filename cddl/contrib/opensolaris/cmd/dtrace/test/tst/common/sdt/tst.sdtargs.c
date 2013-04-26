
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
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <unistd.h>
#ifndef __FreeBSD__
#include <sys/uadmin.h>
#endif

int
main(int argc, char **argv)
{
#ifdef __FreeBSD__
	return (1);
#else
	while (1) {
		if (uadmin(A_SDTTEST, 0, 0) < 0) {
			perror("uadmin");
			return (1);
		}

		sleep(1);
	}

	return (0);
#endif
}