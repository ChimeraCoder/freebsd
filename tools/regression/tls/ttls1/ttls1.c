
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
/* $FreeBSD$ */
#include <stdio.h>

extern int __thread xx1;
extern int __thread xx2;
extern int __thread xxa[];
int __thread a[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
extern int xxyy();

int main(int argc, char** argv)
{
	printf("xx1=%d, xx2=%d, xxa[5]=%d, a[5]=%d, xxyy()=%d\n",
	    xx1, xx2, xxa[5], a[5], xxyy());
}