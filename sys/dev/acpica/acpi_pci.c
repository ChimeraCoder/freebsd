
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
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include <dev/acpica/acpivar.h>

#include <sys/pciio.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pci_private.h>

#include "pcib_if.h"
#include "pci_if.h"

/* Hooks for the ACPI CA debugging infrastructure. */
#define _COMPONENT	ACPI_BUS
ACPI_MODULE_NAME("PCI")

struct acpi_pci_devinfo {
	struct pci_devinfo	ap_dinfo;
	ACPI_HANDLE		ap_handle;
	int			ap_flags;
};

ACPI_SERIAL_DECL(pci_powerstate, "ACPI PCI power methods");

/* Be sure that ACPI and PCI power states are equivalent. */
CTASSERT(ACPI_STATE_D0 == PCI_POWERSTATE_D0);
CTASSERT(ACPI_STATE_D1 == PCI_POWERSTATE_D1);
CTASSERT(ACPI_STATE_D2 == PCI_POWERSTATE_D2);
CTASSERT(ACPI_STATE_D3 == PCI_POWERSTATE_D3);

static int	acpi_pci_attach(device_t dev);
static int	acpi_pci_child_location_str_method(device_t cbdev,
		    device_t child, char *buf, size_t buflen);
static int	acpi_pci_probe(device_t dev);
static int	acpi_pci_read_ivar(device_t dev, device_t child, int which,
		    uintptr_t *result);
static int	acpi_pci_write_ivar(device_t dev, device_t child, int which,
		    uintptr_t value);
static ACPI_STATUS acpi_pci_save_handle(ACPI_HANDLE handle, UINT32 level,
		    void *context, void **status);
static int	acpi_pci_set_powerstate_method(device_t dev, device_t child,
		    int state);
static void	acpi_pci_update_device(ACPI_HANDLE handle, device_t pci_child);

static device_method_t acpi_pci_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		acpi_pci_probe),
	DEVMETHOD(device_attach,	acpi_pci_attach),

	/* Bus interface */
	DEVMETHOD(bus_read_ivar,	acpi_pci_read_ivar),
	DEVMETHOD(bus_write_ivar,	acpi_pci_write_ivar),
	DEVMETHOD(bus_child_location_str, acpi_pci_child_location_str_method),

	/* PCI interface */
	DEVMETHOD(pci_set_powerstate,	acpi_pci_set_powerstate_method),

	DEVMETHOD_END
};

static devclass_t pci_devclass;

DEFINE_CLASS_1(pci, acpi_pci_driver, acpi_pci_methods, sizeof(struct pci_softc),
    pci_driver);
DRIVER_MODULE(acpi_pci, pcib, acpi_pci_driver, pci_devclass, 0, 0);
MODULE_DEPEND(acpi_pci, acpi, 1, 1, 1);
MODULE_DEPEND(acpi_pci, pci, 1, 1, 1);
MODULE_VERSION(acpi_pci, 1);

static int
acpi_pci_read_ivar(device_t dev, device_t child, int which, uintptr_t *result)
{
    struct acpi_pci_devinfo *dinfo;

    dinfo = device_get_ivars(child);
    switch (which) {
    case ACPI_IVAR_HANDLE:
	*result = (uintptr_t)dinfo->ap_handle;
	return (0);
    case ACPI_IVAR_FLAGS:
	*result = (uintptr_t)dinfo->ap_flags;
	return (0);
    }
    return (pci_read_ivar(dev, child, which, result));
}

static int
acpi_pci_write_ivar(device_t dev, device_t child, int which, uintptr_t value)
{
    struct acpi_pci_devinfo *dinfo;

    dinfo = device_get_ivars(child);
    switch (which) {
    case ACPI_IVAR_HANDLE:
	dinfo->ap_handle = (ACPI_HANDLE)value;
	return (0);
    case ACPI_IVAR_FLAGS:
	dinfo->ap_flags = (int)value;
	return (0);
    }
    return (pci_write_ivar(dev, child, which, value));
}

static int
acpi_pci_child_location_str_method(device_t cbdev, device_t child, char *buf,
    size_t buflen)
{
    struct acpi_pci_devinfo *dinfo = device_get_ivars(child);

    pci_child_location_str_method(cbdev, child, buf, buflen);
    
    if (dinfo->ap_handle) {
	strlcat(buf, " handle=", buflen);
	strlcat(buf, acpi_name(dinfo->ap_handle), buflen);
    }
    return (0);
}

/*
 * PCI power manangement
 */
static int
acpi_pci_set_powerstate_method(device_t dev, device_t child, int state)
{
	ACPI_HANDLE h;
	ACPI_STATUS status;
	int old_state, error;

	error = 0;
	if (state < ACPI_STATE_D0 || state > ACPI_STATE_D3)
		return (EINVAL);

	/*
	 * We set the state using PCI Power Management outside of setting
	 * the ACPI state.  This means that when powering down a device, we
	 * first shut it down using PCI, and then using ACPI, which lets ACPI
	 * try to power down any Power Resources that are now no longer used.
	 * When powering up a device, we let ACPI set the state first so that
	 * it can enable any needed Power Resources before changing the PCI
	 * power state.
	 */
	ACPI_SERIAL_BEGIN(pci_powerstate);
	old_state = pci_get_powerstate(child);
	if (old_state < state && pci_do_power_suspend) {
		error = pci_set_powerstate_method(dev, child, state);
		if (error)
			goto out;
	}
	h = acpi_get_handle(child);
	status = acpi_pwr_switch_consumer(h, state);
	if (ACPI_SUCCESS(status)) {
		if (bootverbose)
			device_printf(dev, "set ACPI power state D%d on %s\n",
			    state, acpi_name(h));
	} else if (status != AE_NOT_FOUND)
		device_printf(dev,
		    "failed to set ACPI power state D%d on %s: %s\n",
		    state, acpi_name(h), AcpiFormatException(status));
	if (old_state > state && pci_do_power_resume)
		error = pci_set_powerstate_method(dev, child, state);

out:
	ACPI_SERIAL_END(pci_powerstate);
	return (error);
}

static void
acpi_pci_update_device(ACPI_HANDLE handle, device_t pci_child)
{
	ACPI_STATUS status;
	device_t child;

	/*
	 * Occasionally a PCI device may show up as an ACPI device
	 * with a _HID.  (For example, the TabletPC TC1000 has a
	 * second PCI-ISA bridge that has a _HID for an
	 * acpi_sysresource device.)  In that case, leave ACPI-CA's
	 * device data pointing at the ACPI-enumerated device.
	 */
	child = acpi_get_device(handle);
	if (child != NULL) {
		KASSERT(device_get_parent(child) ==
		    devclass_get_device(devclass_find("acpi"), 0),
		    ("%s: child (%s)'s parent is not acpi0", __func__,
		    acpi_name(handle)));
		return;
	}

	/*
	 * Update ACPI-CA to use the PCI enumerated device_t for this handle.
	 */
	status = AcpiAttachData(handle, acpi_fake_objhandler, pci_child);
	if (ACPI_FAILURE(status))
		printf("WARNING: Unable to attach object data to %s - %s\n",
		    acpi_name(handle), AcpiFormatException(status));
}

static ACPI_STATUS
acpi_pci_save_handle(ACPI_HANDLE handle, UINT32 level, void *context,
    void **status)
{
	struct acpi_pci_devinfo *dinfo;
	device_t *devlist;
	int devcount, i, func, slot;
	UINT32 address;

	ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

	if (ACPI_FAILURE(acpi_GetInteger(handle, "_ADR", &address)))
		return_ACPI_STATUS (AE_OK);
	slot = ACPI_ADR_PCI_SLOT(address);
	func = ACPI_ADR_PCI_FUNC(address);
	if (device_get_children((device_t)context, &devlist, &devcount) != 0)
		return_ACPI_STATUS (AE_OK);
	for (i = 0; i < devcount; i++) {
		dinfo = device_get_ivars(devlist[i]);
		if (dinfo->ap_dinfo.cfg.func == func &&
		    dinfo->ap_dinfo.cfg.slot == slot) {
			dinfo->ap_handle = handle;
			acpi_pci_update_device(handle, devlist[i]);
			break;
		}
	}
	free(devlist, M_TEMP);
	return_ACPI_STATUS (AE_OK);
}

static int
acpi_pci_probe(device_t dev)
{

	if (acpi_get_handle(dev) == NULL)
		return (ENXIO);
	device_set_desc(dev, "ACPI PCI bus");
	return (0);
}

static int
acpi_pci_attach(device_t dev)
{
	int busno, domain, error;

	error = pci_attach_common(dev);
	if (error)
		return (error);

	/*
	 * Since there can be multiple independantly numbered PCI
	 * busses on systems with multiple PCI domains, we can't use
	 * the unit number to decide which bus we are probing. We ask
	 * the parent pcib what our domain and bus numbers are.
	 */
	domain = pcib_get_domain(dev);
	busno = pcib_get_bus(dev);

	/*
	 * First, PCI devices are added as in the normal PCI bus driver.
	 * Afterwards, the ACPI namespace under the bridge driver is
	 * walked to save ACPI handles to all the devices that appear in
	 * the ACPI namespace as immediate descendants of the bridge.
	 *
	 * XXX: Sometimes PCI devices show up in the ACPI namespace that
	 * pci_add_children() doesn't find.  We currently just ignore
	 * these devices.
	 */
	pci_add_children(dev, domain, busno, sizeof(struct acpi_pci_devinfo));
	AcpiWalkNamespace(ACPI_TYPE_DEVICE, acpi_get_handle(dev), 1,
	    acpi_pci_save_handle, NULL, dev, NULL);

	return (bus_generic_attach(dev));
}