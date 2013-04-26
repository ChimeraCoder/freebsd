
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
 * Test for heapsort() routine.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "test-sort.h"

int
main(int argc, char *argv[])
{
  int i, j;
  int testvector[IVEC_LEN];
  int sresvector[IVEC_LEN];

  printf("1..1\n");
  for (j = 2; j < IVEC_LEN; j++) {
    /* Populate test vectors */
    for (i = 0; i < j; i++)
      testvector[i] = sresvector[i] = initvector[i];

    /* Sort using heapsort(3) */
    heapsort(testvector, j, sizeof(testvector[0]), sorthelp);
    /* Sort using reference slow sorting routine */
    ssort(sresvector, j);

    /* Compare results */
    for (i = 0; i < j; i++)
      assert(testvector[i] == sresvector[i]);
  }

  printf("ok 1 - heapsort\n");

  return(0);
}