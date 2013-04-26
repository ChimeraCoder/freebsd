
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/proc.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>

#include <ddb/ddb.h>
#include <ddb/db_watch.h>

/*
 * Watchpoints.
 */

static boolean_t	db_watchpoints_inserted = TRUE;

#define	NWATCHPOINTS	100
static struct db_watchpoint	db_watch_table[NWATCHPOINTS];
static db_watchpoint_t	db_next_free_watchpoint = &db_watch_table[0];
static db_watchpoint_t	db_free_watchpoints = 0;
static db_watchpoint_t	db_watchpoint_list = 0;

static db_watchpoint_t	db_watchpoint_alloc(void);
static void		db_watchpoint_free(db_watchpoint_t watch);
static void		db_delete_watchpoint(vm_map_t map, db_addr_t addr);
#ifdef notused
static boolean_t	db_find_watchpoint(vm_map_t map, db_addr_t addr,
					db_regs_t *regs);
#endif
static void		db_list_watchpoints(void);
static void		db_set_watchpoint(vm_map_t map, db_addr_t addr,
				       vm_size_t size);

static db_watchpoint_t
db_watchpoint_alloc()
{
	register db_watchpoint_t	watch;

	if ((watch = db_free_watchpoints) != 0) {
	    db_free_watchpoints = watch->link;
	    return (watch);
	}
	if (db_next_free_watchpoint == &db_watch_table[NWATCHPOINTS]) {
	    db_printf("All watchpoints used.\n");
	    return (0);
	}
	watch = db_next_free_watchpoint;
	db_next_free_watchpoint++;

	return (watch);
}

static void
db_watchpoint_free(watch)
	register db_watchpoint_t	watch;
{
	watch->link = db_free_watchpoints;
	db_free_watchpoints = watch;
}

static void
db_set_watchpoint(map, addr, size)
	vm_map_t	map;
	db_addr_t	addr;
	vm_size_t	size;
{
	register db_watchpoint_t	watch;

	if (map == NULL) {
	    db_printf("No map.\n");
	    return;
	}

	/*
	 *	Should we do anything fancy with overlapping regions?
	 */

	for (watch = db_watchpoint_list;
	     watch != 0;
	     watch = watch->link)
	    if (db_map_equal(watch->map, map) &&
		(watch->loaddr == addr) &&
		(watch->hiaddr == addr+size)) {
		db_printf("Already set.\n");
		return;
	    }

	watch = db_watchpoint_alloc();
	if (watch == 0) {
	    db_printf("Too many watchpoints.\n");
	    return;
	}

	watch->map = map;
	watch->loaddr = addr;
	watch->hiaddr = addr+size;

	watch->link = db_watchpoint_list;
	db_watchpoint_list = watch;

	db_watchpoints_inserted = FALSE;
}

static void
db_delete_watchpoint(map, addr)
	vm_map_t	map;
	db_addr_t	addr;
{
	register db_watchpoint_t	watch;
	register db_watchpoint_t	*prev;

	for (prev = &db_watchpoint_list;
	     (watch = *prev) != 0;
	     prev = &watch->link)
	    if (db_map_equal(watch->map, map) &&
		(watch->loaddr <= addr) &&
		(addr < watch->hiaddr)) {
		*prev = watch->link;
		db_watchpoint_free(watch);
		return;
	    }

	db_printf("Not set.\n");
}

static void
db_list_watchpoints()
{
	register db_watchpoint_t	watch;

	if (db_watchpoint_list == 0) {
	    db_printf("No watchpoints set\n");
	    return;
	}

#ifdef __LP64__
	db_printf(" Map                Address          Size\n");
#else
	db_printf(" Map        Address  Size\n");
#endif
	for (watch = db_watchpoint_list;
	     watch != 0;
	     watch = watch->link)
#ifdef __LP64__
	    db_printf("%s%16p  %16lx  %lx\n",
#else
	    db_printf("%s%8p  %8lx  %lx\n",
#endif
		      db_map_current(watch->map) ? "*" : " ",
		      (void *)watch->map, (long)watch->loaddr,
		      (long)watch->hiaddr - (long)watch->loaddr);
}

/* Delete watchpoint */
/*ARGSUSED*/
void
db_deletewatch_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char *		modif;
{
	db_delete_watchpoint(db_map_addr(addr), addr);
}

/* Set watchpoint */
/*ARGSUSED*/
void
db_watchpoint_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char *		modif;
{
	vm_size_t	size;
	db_expr_t	value;

	if (db_expression(&value))
	    size = (vm_size_t) value;
	else
	    size = 4;
	db_skip_to_eol();

	db_set_watchpoint(db_map_addr(addr), addr, size);
}

/*
 * At least one non-optional show-command must be implemented using
 * DB_SHOW_COMMAND() so that db_show_cmd_set gets created.  Here is one.
 */
DB_SHOW_COMMAND(watches, db_listwatch_cmd)
{
	db_list_watchpoints();
	db_md_list_watchpoints();
}

void
db_set_watchpoints()
{
	register db_watchpoint_t	watch;

	if (!db_watchpoints_inserted) {
	    for (watch = db_watchpoint_list;
	         watch != 0;
	         watch = watch->link)
		pmap_protect(watch->map->pmap,
			     trunc_page(watch->loaddr),
			     round_page(watch->hiaddr),
			     VM_PROT_READ);

	    db_watchpoints_inserted = TRUE;
	}
}

void
db_clear_watchpoints()
{
	db_watchpoints_inserted = FALSE;
}

#ifdef notused
static boolean_t
db_find_watchpoint(map, addr, regs)
	vm_map_t	map;
	db_addr_t	addr;
	db_regs_t	*regs;
{
	register db_watchpoint_t watch;
	db_watchpoint_t found = 0;

	for (watch = db_watchpoint_list;
	     watch != 0;
	     watch = watch->link)
	    if (db_map_equal(watch->map, map)) {
		if ((watch->loaddr <= addr) &&
		    (addr < watch->hiaddr))
		    return (TRUE);
		else if ((trunc_page(watch->loaddr) <= addr) &&
			 (addr < round_page(watch->hiaddr)))
		    found = watch;
	    }

	/*
	 *	We didn't hit exactly on a watchpoint, but we are
	 *	in a protected region.  We want to single-step
	 *	and then re-protect.
	 */

	if (found) {
	    db_watchpoints_inserted = FALSE;
	    db_single_step(regs);
	}

	return (FALSE);
}
#endif



/* Delete hardware watchpoint */
/*ARGSUSED*/
void
db_deletehwatch_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char *		modif;
{
	int rc;

        if (count < 0)
                count = 4;

	rc = db_md_clr_watchpoint(addr, count);
	if (rc < 0)
		db_printf("hardware watchpoint could not be deleted\n");
}

/* Set hardware watchpoint */
/*ARGSUSED*/
void
db_hwatchpoint_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char *		modif;
{
	int rc;

        if (count < 0)
                count = 4;

	rc = db_md_set_watchpoint(addr, count);
	if (rc < 0)
		db_printf("hardware watchpoint could not be set\n");
}