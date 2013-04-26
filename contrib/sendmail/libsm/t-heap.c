
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

#include <sm/gen.h>
SM_IDSTR(id, "@(#)$Id: t-heap.c,v 1.10 2001/09/11 04:04:49 gshapiro Exp $")

#include <sm/debug.h>
#include <sm/heap.h>
#include <sm/io.h>
#include <sm/test.h>
#include <sm/xtrap.h>

#if SM_HEAP_CHECK
extern SM_DEBUG_T SmHeapCheck;
# define HEAP_CHECK sm_debug_active(&SmHeapCheck, 1)
#else /* SM_HEAP_CHECK */
# define HEAP_CHECK 0
#endif /* SM_HEAP_CHECK */

int
main(argc, argv)
	int argc;
	char **argv;
{
	void *p;

	sm_test_begin(argc, argv, "test heap handling");
	if (argc > 1)
		sm_debug_addsettings_x(argv[1]);

	p = sm_malloc(10);
	SM_TEST(p != NULL);
	p = sm_realloc_x(p, 20);
	SM_TEST(p != NULL);
	p = sm_realloc(p, 30);
	SM_TEST(p != NULL);
	if (HEAP_CHECK)
	{
		sm_dprintf("heap with 1 30-byte block allocated:\n");
		sm_heap_report(smioout, 3);
	}

	if (HEAP_CHECK)
	{
		sm_free(p);
		sm_dprintf("heap with 0 blocks allocated:\n");
		sm_heap_report(smioout, 3);
		sm_dprintf("xtrap count = %d\n", SmXtrapCount);
	}

#if DEBUG
	/* this will cause a core dump */
	sm_dprintf("about to free %p for the second time\n", p);
	sm_free(p);
#endif /* DEBUG */

	return sm_test_end();
}