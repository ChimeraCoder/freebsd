
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
 * This works on:
 *   Crays
 *   Conven
 *   sparc's
 *   Dec mip machines
 *   Dec alpha machines
 *   RS6000
 *   SGI's
 */
#include <stdio.h>

int
main(
	int argc,
	char *argv[]
	)
{
	int i;
	int big;
	union {
		unsigned long l;
		char c[sizeof(long)];
	} u;

#if defined(LONG8)
	u.l = (((long)0x08070605) << 32) | (long)0x04030201;
#else
	u.l = 0x04030201;
#endif
	if (sizeof(long) > 4) {
		if (u.c[0] == 0x08) big = 1;
		else		    big = 0;
	} else {
		if (u.c[0] == 0x04) big = 1;
		else		    big = 0;
	}
	for (i=0; i< sizeof(long); i++) {
		if (big == 1 && (u.c[i] == (sizeof(long) - i))) { 
			continue; 
		} else if (big == 0 && (u.c[i] == (i+1))) {
			continue;
		} else {
			big = -1;
			break;
		}
	}

	if (big == 1) {
		printf("XNTP_BIG_ENDIAN\n");
	} else if (big == 0) {
		printf("XNTP_LITTLE_ENDIAN\n");
	}
	exit(0);
}