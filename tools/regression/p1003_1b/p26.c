
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
#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#include <stdio.h>

int p26(int ac, char *av[])
{
	int ret = 0;

	#ifndef _POSIX_VERSION
	printf("POSIX is not supported.\n");
	ret = -1;
	#else	/* _POSIX_VERSION */

	#if (_POSIX_VERSION == 198808L)
	printf("POSIX.1 is supported but not POSIX.1B (FIPS 151-1)\n");
	#elif (_POSIX_VERSION == 199009L)
	printf("POSIX.1 is supported but not POSIX.1B (FIPS 151-2)\n");
	#elif (_POSIX_VERSION >= 199309L)
	printf("POSIX.1 and POSIX.1B are supported.\n");
	#else
	printf("_POSIX_VERSION (%ld) not 198808, 199009, or >= 199309.\n",
		_POSIX_VERSION);
	ret = -1;
	#endif

	#endif	/* _POSIX_VERSION */
	return ret;
}
#ifdef STANDALONE_TESTS
int main(int argc, char *argv[]) { return p26(argc, argv); }
#endif