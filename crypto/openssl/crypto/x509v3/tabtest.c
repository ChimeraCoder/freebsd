
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

/* Simple program to check the ext_dat.h is correct and print out
 * problems if it is not.
 */

#include <stdio.h>

#include <openssl/x509v3.h>

#include "ext_dat.h"

main()
{
	int i, prev = -1, bad = 0;
	X509V3_EXT_METHOD **tmp;
	i = sizeof(standard_exts) / sizeof(X509V3_EXT_METHOD *);
	if(i != STANDARD_EXTENSION_COUNT)
		fprintf(stderr, "Extension number invalid expecting %d\n", i);
	tmp = standard_exts;
	for(i = 0; i < STANDARD_EXTENSION_COUNT; i++, tmp++) {
		if((*tmp)->ext_nid < prev) bad = 1;
		prev = (*tmp)->ext_nid;
		
	}
	if(bad) {
		tmp = standard_exts;
		fprintf(stderr, "Extensions out of order!\n");
		for(i = 0; i < STANDARD_EXTENSION_COUNT; i++, tmp++)
		printf("%d : %s\n", (*tmp)->ext_nid, OBJ_nid2sn((*tmp)->ext_nid));
	} else fprintf(stderr, "Order OK\n");
}