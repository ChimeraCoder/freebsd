
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
 * Miscellaneous printing.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kdb.h>
#include <sys/proc.h>

#include <machine/pcb.h>

#include <ddb/ddb.h>
#include <ddb/db_variables.h>
#include <ddb/db_sym.h>

void
db_show_regs(db_expr_t _1, boolean_t _2, db_expr_t _3, char *_4)
{
	struct db_variable *regp;
	db_expr_t value, offset;
	const char *name;

	for (regp = db_regs; regp < db_eregs; regp++) {
		if (!db_read_variable(regp, &value))
			continue;
		db_printf("%-12s%#10lr", regp->name, (unsigned long)value);
		db_find_xtrn_sym_and_offset((db_addr_t)value, &name, &offset);
		if (name != NULL && offset <= (unsigned long)db_maxoff &&
		    offset != value) {
			db_printf("\t%s", name);
			if (offset != 0)
				db_printf("+%+#lr", (long)offset);
		}
		db_printf("\n");
	}
	db_print_loc_and_inst(PC_REGS());
}