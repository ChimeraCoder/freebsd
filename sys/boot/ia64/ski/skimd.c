
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

#include <libia64.h>

#include "libski.h"

extern void acpi_stub_init(void);
extern void efi_stub_init(struct bootinfo *);
extern void sal_stub_init(void);

vm_paddr_t
ia64_platform_alloc(vm_offset_t va, vm_size_t sz __unused)
{
	vm_paddr_t pa;

	if (va == 0)
		pa = 2 * 1024 * 1024;
	else
		pa = (va - IA64_PBVM_BASE) + (32 * 1024 * 1024);

	return (pa);
}

void
ia64_platform_free(vm_offset_t va __unused, vm_paddr_t pa __unused,
    vm_size_t sz __unused)
{
}

int
ia64_platform_bootinfo(struct bootinfo *bi, struct bootinfo **res)
{
	static struct bootinfo bootinfo;

	efi_stub_init(bi);
	sal_stub_init();
	acpi_stub_init();

	if (IS_LEGACY_KERNEL())
		*res = &bootinfo;

	return (0);
}

int
ia64_platform_enter(const char *kernel)
{

	while (*kernel == '/')
		kernel++;
	ssc(0, (uint64_t)kernel, 0, 0, SSC_LOAD_SYMBOLS);
	return (0);
}