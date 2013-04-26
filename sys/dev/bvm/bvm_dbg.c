
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
#include <sys/bus.h>

#include <gdb/gdb.h>

#include <machine/cpufunc.h>

static gdb_probe_f bvm_dbg_probe;
static gdb_init_f bvm_dbg_init;
static gdb_term_f bvm_dbg_term;
static gdb_getc_f bvm_dbg_getc;
static gdb_putc_f bvm_dbg_putc;

GDB_DBGPORT(bvm, bvm_dbg_probe, bvm_dbg_init, bvm_dbg_term,
    bvm_dbg_getc, bvm_dbg_putc);

#define	BVM_DBG_PORT	0x224
static int bvm_dbg_port = BVM_DBG_PORT;

#define BVM_DBG_SIG	('B' << 8 | 'V')

static int
bvm_dbg_probe(void)
{
	int disabled, port;

	disabled = 0;
	resource_int_value("bvmdbg", 0, "disabled", &disabled);

	if (!disabled) {
		if (resource_int_value("bvmdbg", 0, "port", &port) == 0)
			bvm_dbg_port = port;

		if (inw(bvm_dbg_port) == BVM_DBG_SIG) {
			/*
			 * Return a higher priority than 0 to override other
			 * gdb dbgport providers that may be present (e.g. uart)
			 */
			return (1);
		}
	}

	return (-1);
}

static void
bvm_dbg_init(void)
{
}

static void
bvm_dbg_term(void)
{
}

static void
bvm_dbg_putc(int c)
{

	outl(bvm_dbg_port, c);
}

static int
bvm_dbg_getc(void)
{

	return (inl(bvm_dbg_port));
}