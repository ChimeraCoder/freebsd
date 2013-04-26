
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
**  This program checks to see if your version of seteuid works.
**  Compile it, make it set-user-ID root, and run it as yourself (NOT as
**  root).  If it won't compile or outputs any MAYDAY messages, don't
**  define USESETEUID in conf.h.
**
**	NOTE:  It is not sufficient to have seteuid in your library.
**	You must also have saved uids that function properly.
**
**  Compilation is trivial -- just "cc t_seteuid.c".  Make it set-user-ID
**  root and then execute it as a non-root user.
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#ifndef lint
static char id[] = "@(#)$Id: t_seteuid.c,v 8.8 2001/09/23 03:35:41 ca Exp $";
#endif /* ! lint */

#ifdef __hpux
# define seteuid(e)	setresuid(-1, e, -1)
#endif /* __hpux */

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

	if (seteuid(1) < 0)
		printf("seteuid(1) failure\n");
	printuids("after seteuid(1)", realuid, 1);

	if (geteuid() != 1)
	{
		fail++;
		printf("MAYDAY!  Wrong effective uid\n");
	}

	/* do activity here */

	if (seteuid(0) < 0)
	{
		fail++;
		printf("seteuid(0) failure\n");
	}
	printuids("after seteuid(0)", realuid, 0);

	if (geteuid() != 0)
	{
		fail++;
		printf("MAYDAY!  Wrong effective uid\n");
	}
	if (getuid() != realuid)
	{
		fail++;
		printf("MAYDAY!  Wrong real uid\n");
	}
	printf("\n");

	if (seteuid(2) < 0)
	{
		fail++;
		printf("seteuid(2) failure\n");
	}
	printuids("after seteuid(2)", realuid, 2);

	if (geteuid() != 2)
	{
		fail++;
		printf("MAYDAY!  Wrong effective uid\n");
	}

	/* do activity here */

	if (seteuid(0) < 0)
	{
		fail++;
		printf("seteuid(0) failure\n");
	}
	printuids("after seteuid(0)", realuid, 0);

	if (geteuid() != 0)
	{
		fail++;
		printf("MAYDAY!  Wrong effective uid\n");
	}
	if (getuid() != realuid)
	{
		fail++;
		printf("MAYDAY!  Wrong real uid\n");
	}

	if (fail)
	{
		printf("\nThis system cannot use seteuid\n");
		exit(1);
	}

	printf("\nIt is safe to define USESETEUID on this system\n");
	exit(0);
}