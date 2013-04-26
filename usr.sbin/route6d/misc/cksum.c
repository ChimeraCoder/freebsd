
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

unsigned short buf[BUFSIZ];

main()
{
	int	i;
	unsigned short *p = buf, *q = &buf[4];
	unsigned long sum, sum2;

	while (scanf("%x", &i) != EOF) {
		*p++ = i; printf("%d ", i);
	}
	printf("\n");

	sum = buf[2] + (buf[3] >> 8) & 0xff;
	while (q != p)
		sum += (*q++ & 0xffff);
	sum2 = (sum & 0xffff) + (sum >> 16) & 0xffff;
	printf("%x, %x\n", sum, sum2);
}