
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

#include <stand.h>
#include <efi.h>
#include <efilib.h>

/*
 * We could use linker sets for some or all of these, but
 * then we would have to control what ended up linked into
 * the bootstrap.  So it's easier to conditionalise things
 * here.
 *
 * XXX rename these arrays to be consistent and less namespace-hostile
 */

/* Exported for libstand */
struct devsw *devsw[] = {
	&efipart_dev,
	&efinet_dev,
	NULL
};

struct fs_ops *file_system[] = {
	&dosfs_fsops,
	&ufs_fsops,
	&cd9660_fsops,
	&nfs_fsops,
	&gzipfs_fsops,
	NULL
};

struct netif_driver *netif_drivers[] = {
	&efinetif,
	NULL
};

/* 
 * Consoles 
 *
 * We don't prototype these in efiboot.h because they require
 * data structures from bootstrap.h as well.
 */
extern struct console efi_console;

struct console *consoles[] = {
	&efi_console,
	NULL
};