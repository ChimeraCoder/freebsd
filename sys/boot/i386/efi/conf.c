
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
#include <bootstrap.h>
#include <efi.h>
#include <efilib.h>

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

#ifdef notyet
extern struct file_format amd64_elf;
extern struct file_format amd64_elf_obj;
#endif
extern struct file_format i386_elf;
extern struct file_format i386_elf_obj;

struct file_format *file_formats[] = {
#ifdef notyet
	&amd64_elf,
	&amd64_elf_obj,
#endif
	&i386_elf,
	&i386_elf_obj,
	NULL
};

extern struct console efi_console;

struct console *consoles[] = {
	&efi_console,
	NULL
};