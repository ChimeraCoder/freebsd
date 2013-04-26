
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
/* $FreeBSD$ */#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

u_long NBUCKETS		= 2000;
u_long NOPS		= 200000;
u_long NSIZE		= (16*1024);

char **foo;

int
main(int argc, char **argv) 
{
    u_long i,j,k;
    
    if (argc > 1) NOPS     = strtoul(argv[1],0,0);
    if (argc > 2) NBUCKETS = strtoul(argv[2],0,0);
    if (argc > 3) NSIZE	   = strtoul(argv[3],0,0);
    printf("BRK(0)=%p ", sbrk(0));
    foo = malloc(sizeof(*foo) * NBUCKETS);
    memset(foo, 0, sizeof(*foo) * NBUCKETS);
    for (i = 1; i <= 4096; i *= 2) {
        for (j = 0; j < 40960/i && j < NBUCKETS; j++) {
	    foo[j] = malloc(i);
        }
        for (j = 0; j < 40960/i && j < NBUCKETS; j++) {
	    free(foo[j]);
	    foo[j] = NULL;
        }
    }

    for (i = 0; i < NOPS; i++) {
	j = random() % NBUCKETS;
	k = random() % NSIZE;
	foo[j] = realloc(foo[j], k & 1 ? 0 : k);
	if (k & 1 || k == 0) {
		/*
		 * Workaround because realloc return bogus pointer rather than
		 * NULL if passed zero length.
		 */
		foo[j] = NULL;
	}
	if (foo[j])
	    foo[j][0] = 1;
    }
    printf("BRK(1)=%p ", sbrk(0));
    for (j = 0; j < NBUCKETS; j++) {
	if (foo[j]) {
	    free(foo[j]);
	    foo[j] = NULL;
	}
    }
    printf("BRK(2)=%p NOPS=%lu NBUCKETS=%lu NSIZE=%lu\n",
	sbrk(0), NOPS, NBUCKETS, NSIZE);
    return 0;
}