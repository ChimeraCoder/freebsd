
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

#include "ipf.h"

#define	PRINTF	(void)printf
#define	FPRINTF	(void)fprintf

void printpooldata(pool, opts)
ip_pool_t *pool;
int opts;
{

	if ((opts & OPT_DEBUG) == 0) {
		if ((pool->ipo_flags & IPOOL_ANON) != 0)
			PRINTF("# 'anonymous' tree %s\n", pool->ipo_name);
		if ((pool->ipo_flags & IPOOL_DELETE) != 0)
			PRINTF("# ");
		PRINTF("table role = ");
	} else {
		if ((pool->ipo_flags & IPOOL_DELETE) != 0)
			PRINTF("# ");
		PRINTF("%s: %s",
			isdigit(*pool->ipo_name) ? "Number" : "Name",
			pool->ipo_name);
		if ((pool->ipo_flags & IPOOL_ANON) == IPOOL_ANON)
			PRINTF("(anon)");
		putchar(' ');
		PRINTF("Role: ");
	}

	switch (pool->ipo_unit)
	{
	case IPL_LOGIPF :
		printf("ipf");
		break;
	case IPL_LOGNAT :
		printf("nat");
		break;
	case IPL_LOGSTATE :
		printf("state");
		break;
	case IPL_LOGAUTH :
		printf("auth");
		break;
	case IPL_LOGSYNC :
		printf("sync");
		break;
	case IPL_LOGSCAN :
		printf("scan");
		break;
	case IPL_LOGLOOKUP :
		printf("lookup");
		break;
	case IPL_LOGCOUNT :
		printf("count");
		break;
	default :
		printf("unknown(%d)", pool->ipo_unit);
	}

	if ((opts & OPT_DEBUG) == 0) {
		PRINTF(" type = tree %s = %s\n",
			isdigit(*pool->ipo_name) ? "number" : "name",
			pool->ipo_name);
	} else {
		putchar(' ');

		PRINTF("\tReferences: %d\tHits: %lu\n", pool->ipo_ref,
			pool->ipo_hits);
		if ((pool->ipo_flags & IPOOL_DELETE) != 0)
			PRINTF("# ");
		PRINTF("\tNodes Starting at %p\n", pool->ipo_list);
	}
}