
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

#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>

#include "scenario.h"

int
main(int argc, char *argv[])
{
	int error;

	fprintf(stderr, "test capabilities: ");
#ifdef SETSUGID_SUPPORTED
	fprintf(stderr, "[SETSUGID_SUPPORTED] ");
#endif
#ifdef SETSUGID_SUPPORTED_BUT_NO_LIBC_STUB
	fprintf(stderr, "[SETSUGID_SUPPORTED_BUT_NO_LIBC_STUB] ");
#endif
#ifdef CHECK_CRED_SET
	fprintf(stderr, "[CHECK_CRED_SET] ");
#endif
	fprintf(stderr, "\n");

	error = setugid(1);
	if (error) {
		perror("setugid");
		fprintf(stderr,
		    "This test suite requires options REGRESSION\n");
		return (-1);
	}

	enact_scenarios();

	return (0);
}