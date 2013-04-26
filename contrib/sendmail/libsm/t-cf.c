
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
SM_IDSTR(id, "@(#)$Id: t-cf.c,v 1.7 2001/09/11 04:04:49 gshapiro Exp $")

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sm/cf.h>

int
main(argc, argv)
	int argc;
	char **argv;
{
	SM_CF_OPT_T opt;
	int err;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s .cf-file option\n", argv[0]);
		exit(1);
	}
	opt.opt_name = argv[2];
	opt.opt_val = NULL;
	err = sm_cf_getopt(argv[1], 1, &opt);
	if (err)
	{
		fprintf(stderr, "%s: %s\n", argv[1], strerror(err));
		exit(1);
	}
	if (opt.opt_val == NULL)
		printf("Error: option \"%s\" not found\n", opt.opt_name);
	else
		printf("%s=%s\n", opt.opt_name, opt.opt_val);
	return 0;
}