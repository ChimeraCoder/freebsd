
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
**  This program checks to see if your version of setuid works.
**  Compile it, make it set-user-ID root, and run it as yourself (NOT as
**  root).
**
**	NOTE:  This should work everywhere, but Linux has the ability
**	to use the undocumented setcap() call to make this break.
**
**  Compilation is trivial -- just "cc t_setuid.c".  Make it set-user-ID,
**  root and then execute it as a non-root user.
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#ifndef lint
static char id[] = "@(#)$Id: t_setuid.c,v 8.7 2001/09/23 03:35:41 ca Exp $";
#endif /* ! lint */

static void
printuids(str, r, e)
	char *str;
	uid_t r, e;
{
	printf("%s (should be %d/%d): r/euid=%d/%d\n", str, (int) r, (int) e,
	       (int) getuid(), (int) geteuid());
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	int fail = 0;
	uid_t realuid = getuid();

	printuids("initial uids", realuid, 0);

	if (geteuid() != 0)
	{
		printf("SETUP ERROR: re-run set-user-ID root\n");
		exit(1);
	}

	if (getuid() == 0)
	{
		printf("SETUP ERROR: must be run by a non-root user\n");
		exit(1);
	}

	if (setuid(1) < 0)
		printf("setuid(1) failure\n");
	printuids("after setuid(1)", 1, 1);

	if (geteuid() != 1)
	{
		fail++;
		printf("MAYDAY!  Wrong effective uid\n");
	}

	if (getuid() != 1)
	{
		fail++;
		printf("MAYDAY!  Wrong real uid\n");
	}


	/* do activity here */
	if (setuid(0) == 0)
	{
		fail++;
		printf("MAYDAY!  setuid(0) succeeded (should have failed)\n");
	}
	else
	{
		printf("setuid(0) failed (this is correct)\n");
	}
	printuids("after setuid(0)", 1, 1);

	if (geteuid() != 1)
	{
		fail++;
		printf("MAYDAY!  Wrong effective uid\n");
	}
	if (getuid() != 1)
	{
		fail++;
		printf("MAYDAY!  Wrong real uid\n");
	}
	printf("\n");

	if (fail)
	{
		printf("\nThis system cannot use setuid (maybe use setreuid)\n");
		exit(1);
	}

	printf("\nIt is safe to use setuid on this system\n");
	exit(0);
}