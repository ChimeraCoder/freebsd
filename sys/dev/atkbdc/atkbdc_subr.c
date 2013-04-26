
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

#include "opt_kbd.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/malloc.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/atkbdc/atkbdc_subr.h>
#include <dev/atkbdc/atkbdcreg.h>

MALLOC_DEFINE(M_ATKBDDEV, "atkbddev", "AT Keyboard device");

devclass_t atkbdc_devclass;

int
atkbdc_print_child(device_t bus, device_t dev)
{
	atkbdc_device_t *kbdcdev;
	u_long irq;
	int flags;
	int retval = 0;

	kbdcdev = (atkbdc_device_t *)device_get_ivars(dev);

	retval += bus_print_child_header(bus, dev);
	flags = device_get_flags(dev);
	if (flags != 0)
		retval += printf(" flags 0x%x", flags);
	irq = bus_get_resource_start(dev, SYS_RES_IRQ, kbdcdev->rid);
	if (irq != 0)
		retval += printf(" irq %ld", irq);
	retval += bus_print_child_footer(bus, dev);

	return (retval);
}

int
atkbdc_read_ivar(device_t bus, device_t dev, int index, uintptr_t *val)
{
	atkbdc_device_t *ivar;

	ivar = (atkbdc_device_t *)device_get_ivars(dev);
	switch (index) {
	case KBDC_IVAR_VENDORID:
		*val = (u_long)ivar->vendorid;
		break;
	case KBDC_IVAR_SERIAL:
		*val = (u_long)ivar->serial;
		break;
	case KBDC_IVAR_LOGICALID:
		*val = (u_long)ivar->logicalid;
		break;
	case KBDC_IVAR_COMPATID:
		*val = (u_long)ivar->compatid;
		break;
	default:
		return ENOENT;
	}
	return 0;
}

int
atkbdc_write_ivar(device_t bus, device_t dev, int index, uintptr_t val)
{
	atkbdc_device_t *ivar;

	ivar = (atkbdc_device_t *)device_get_ivars(dev);
	switch (index) {
	case KBDC_IVAR_VENDORID:
		ivar->vendorid = (u_int32_t)val;
		break;
	case KBDC_IVAR_SERIAL:
		ivar->serial = (u_int32_t)val;
		break;
	case KBDC_IVAR_LOGICALID:
		ivar->logicalid = (u_int32_t)val;
		break;
	case KBDC_IVAR_COMPATID:
		ivar->compatid = (u_int32_t)val;
		break;
	default:
		return ENOENT;
	}
	return 0;
}

struct resource_list
*atkbdc_get_resource_list(device_t bus, device_t dev)
{
	atkbdc_device_t *ivar;

	ivar = (atkbdc_device_t *)device_get_ivars(dev);
	return &ivar->resources;
}