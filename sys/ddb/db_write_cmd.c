
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

#include <ddb/ddb.h>
#include <ddb/db_access.h>
#include <ddb/db_command.h>
#include <ddb/db_sym.h>

/*
 * Write to file.
 */
/*ARGSUSED*/
void
db_write_cmd(address, have_addr, count, modif)
	db_expr_t	address;
	boolean_t	have_addr;
	db_expr_t	count;
	char *		modif;
{
	register
	db_addr_t	addr;
	register
	db_expr_t	old_value;
	db_expr_t	new_value;
	register int	size;
	boolean_t	wrote_one = FALSE;

	addr = (db_addr_t) address;

	switch (modif[0]) {
	    case 'b':
		size = 1;
		break;
	    case 'h':
		size = 2;
		break;
	    case 'l':
	    case '\0':
		size = 4;
		break;
	    default:
		db_error("Unknown size\n");
		return;
	}

	while (db_expression(&new_value)) {
	    old_value = db_get_value(addr, size, FALSE);
	    db_printsym(addr, DB_STGY_ANY);
	    db_printf("\t\t%#8lr\t=\t%#8lr\n", (long)old_value,(long)new_value);
	    db_put_value(addr, size, new_value);
	    addr += size;

	    wrote_one = TRUE;
	}

	if (!wrote_one)
	    db_error("Nothing written.\n");

	db_next = addr;
	db_prev = addr - size;

	db_skip_to_eol();
}