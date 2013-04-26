
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
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/eventhandler.h>
#include <sys/reboot.h>
#include <machine/xbox.h>
#include <vm/vm.h>
#include <vm/pmap.h>

#ifndef I686_CPU
#error You must have a I686_CPU in your kernel if you want to make an XBOX-compatible kernel
#endif

static void
xbox_poweroff(void* junk, int howto)
{
	if (!(howto & RB_POWEROFF))
		return;

	pic16l_poweroff();
}

static void
xbox_init(void)
{
	char* ptr;

	if (!arch_i386_is_xbox)
		return;

	/* register our poweroff function */
	EVENTHANDLER_REGISTER (shutdown_final, xbox_poweroff, NULL,
	                       SHUTDOWN_PRI_LAST);

	/*
	 * Some XBOX loaders, such as Cromwell, have a flaw which cause the
	 * nve(4) driver to fail attaching to the NIC.
	 *
	 * This is because they leave the NIC running; this will cause the
	 * Nvidia driver to fail as the NIC does not return any sensible
	 * values and thus fails attaching (using an error 0x5, this means
	 * it cannot find a valid PHY)
	 *
	 * We bluntly tell the NIC to stop whatever it's doing; this makes
	 * nve(4) attach correctly. As the NIC always resides at
	 * 0xfef00000-0xfef003ff on an XBOX, we simply hardcode this address.
	 */
	ptr = pmap_mapdev (0xfef00000, 0x400);
	*(uint32_t*)(ptr + 0x188) = 0; /* clear adapter control field */
	pmap_unmapdev ((vm_offset_t)ptr, 0x400);
}

/*
 * This must be called before the drivers, as the if_nve(4) driver will fail
 * if we do not do this in advance.
 */
SYSINIT(xbox, SI_SUB_DRIVERS, SI_ORDER_FIRST, xbox_init, NULL);