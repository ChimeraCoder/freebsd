
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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <spawn.h>
#include <signal.h>
#include <stdio.h>

void
go(void)
{
	pid_t pid;

	(void) posix_spawn(&pid, "/bin/ls", NULL, NULL, NULL, NULL);

	(void) waitpid(pid, NULL, 0);
}

void
intr(int sig)
{
}

int
main(int argc, char **argv)
{
	struct sigaction sa;

	sa.sa_handler = intr;
	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;

	(void) sigaction(SIGUSR1, &sa, NULL);

	for (;;) {
		go();
	}

	return (0);
}