
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

#include <stdio.h>
#include <sysexits.h>

#ifndef lint
static char id[] = "@(#)$Id: t_snprintf.c,v 8.4 2001/09/23 03:35:41 ca Exp $";
#endif /* ! lint */

#define TEST_STRING	"1234567890"

int
main(argc, argv)
	int argc;
	char **argv;
{
	int r;
	char buf[5];

	r = snprintf(buf, sizeof buf, "%s", TEST_STRING);

	if (buf[sizeof buf - 1] != '\0' ||
	    r != strlen(TEST_STRING))
	{
		fprintf(stderr, "Add the following to devtools/Site/site.config.m4:\n\n");
		fprintf(stderr, "APPENDDEF(`confENVDEF', `-DSNPRINTF_IS_BROKEN=1')\n\n");
		exit(EX_OSERR);
	}
	fprintf(stderr, "snprintf() appears to work properly\n");
	exit(EX_OK);
}