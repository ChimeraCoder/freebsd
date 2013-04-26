
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
 * Prime number generator.  It prints on stdout the next prime number
 * higher than the number specified as argv[1].
 */
#include <stdio.h>
#include <math.h>

main(argc, argv)

int argc;
char *argv[];

{
    double i, j;
    int f;

    if (argc < 2)
    {
	exit(1);
    }

    i = atoi(argv[1]);
    while (i++)
    {
	f=1;
	for (j=2; j<i; j++)
	{
	    if ((i/j)==floor(i/j))
	    {
		f=0;
		break;
	    }
	}
	if (f)
	{
	    printf("%.0f\n", i);
	    exit(0);
	}
    }
}