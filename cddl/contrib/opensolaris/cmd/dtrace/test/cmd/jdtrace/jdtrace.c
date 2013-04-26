
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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <alloca.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/systeminfo.h>

int
main(int argc, char **argv)
{
	int i, ac, has64;
	char **av, **p;

	ac = argc + 3;
	av = p = alloca(sizeof (char *) * ac);

	*p++ = "java";
	*p++ = "-jar";
	*p++ = "/opt/SUNWdtrt/lib/java/jdtrace.jar";

	argc--;
	argv++;

	for (i = 0; i < argc; i++) {
		p[i] = argv[i];
	}
	p[i] = NULL;

	(void) execvp(av[0], av);

	perror("exec failed");

	return (0);
}