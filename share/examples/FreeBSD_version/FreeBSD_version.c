
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
/* $FreeBSD$ */#if __FreeBSD__ == 0		/* 1.0 did not define __FreeBSD__ */
#define __FreeBSD_version 199401
#elif __FreeBSD__ == 1		/* 1.1 defined it to be 1 */
#define __FreeBSD_version 199405
#else				/* 2.0 and higher define it to be 2 */
#include <osreldate.h>		/* and this works */
#endif
#include <stdio.h>
#include <unistd.h>

int
main(void) {
	printf("Compilation release date: %d\n", __FreeBSD_version);
#if __FreeBSD_version >= 199408
	printf("Execution environment release date: %d\n", getosreldate());
#else
	printf("Execution environment release date: can't tell\n");
#endif
	return (0);
}