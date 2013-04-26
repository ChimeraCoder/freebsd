
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

#include "opt_acpi.h"
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include <dev/acpica/acpivar.h>

/* Hooks for the ACPI CA debugging infrastructure */
#define _COMPONENT	ACPI_BUTTON
ACPI_MODULE_NAME("BUTTON")

struct acpi_button_softc {
    device_t	button_dev;
    ACPI_HANDLE	button_handle;
    boolean_t	button_type;
#define		ACPI_POWER_BUTTON	0
#define		ACPI_SLEEP_BUTTON	1
    boolean_t	fixed;
};

#define		ACPI_NOTIFY_BUTTON_PRESSED_FOR_SLEEP	0x80
#define		ACPI_NOTIFY_BUTTON_PRESSED_FOR_WAKEUP	0x02

static int	acpi_button_probe(device_t dev);
static int	acpi_button_attach(device_t dev);
static int	acpi_button_suspend(device_t dev);
static int	acpi_button_resume(device_t dev);
static void 	acpi_button_notify_handler(ACPI_HANDLE h, UINT32 notify,
					   void *context);
static ACPI_STATUS
		acpi_button_fixed_handler(void *context);
static void	acpi_button_notify_sleep(void *arg);
static void	acpi_button_notify_wakeup(void *arg);

static char *btn_ids[] = {
    "PNP0C0C", "ACPI_FPB", "PNP0C0E", "ACPI_FSB",
    NULL
};

static device_method_t acpi_button_methods[] = {
    /* Device interface */
    DEVMETHOD(device_probe,	acpi_button_probe),
    DEVMETHOD(device_attach,	acpi_button_attach),
    DEVMETHOD(device_suspend,	acpi_button_suspend),
    DEVMETHOD(device_shutdown,	acpi_button_suspend),
    DEVMETHOD(device_resume,	acpi_button_resume),
    DEVMETHOD_END
};

static driver_t acpi_button_driver = {
    "acpi_button",
    acpi_button_methods,
    sizeof(struct acpi_button_softc),
};

static devclass_t acpi_button_devclass;
DRIVER_MODULE(acpi_button, acpi, acpi_button_driver, acpi_button_devclass,
	      0, 0);
MODULE_DEPEND(acpi_button, acpi, 1, 1, 1);

static int
acpi_button_probe(device_t dev)
{
    struct acpi_button_softc *sc;
    char *str; 

    if (acpi_disabled("button") ||
	(str = ACPI_ID_PROBE(device_get_parent(dev), dev, btn_ids)) == NULL)
	return (ENXIO);

    sc = device_get_softc(dev);
    if (strcmp(str, "PNP0C0C") == 0) {
	device_set_desc(dev, "Power Button");
	sc->button_type = ACPI_POWER_BUTTON;
    } else if (strcmp(str, "ACPI_FPB") == 0) {
	device_set_desc(dev, "Power Button (fixed)");
	sc->button_type = ACPI_POWER_BUTTON;
	sc->fixed = 1;
    } else if (strcmp(str, "PNP0C0E") == 0) {
	device_set_desc(dev, "Sleep Button");
	sc->button_type = ACPI_SLEEP_BUTTON;
    } else if (strcmp(str, "ACPI_FSB") == 0) {
	device_set_desc(dev, "Sleep Button (fixed)");
	sc->button_type = ACPI_SLEEP_BUTTON;
	sc->fixed = 1;
    }

    return (0);
}

static int
acpi_button_attach(device_t dev)
{
    struct acpi_prw_data	prw;
    struct acpi_button_softc	*sc;
    ACPI_STATUS			status;
    int event;

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    sc = device_get_softc(dev);
    sc->button_dev = dev;
    sc->button_handle = acpi_get_handle(dev);
    event = (sc->button_type == ACPI_SLEEP_BUTTON) ?
	    ACPI_EVENT_SLEEP_BUTTON : ACPI_EVENT_POWER_BUTTON;

    /* 
     * Install the new handler.  We could remove any fixed handlers added
     * from the FADT once we have a duplicate from the AML but some systems
     * only return events on one or the other so we have to keep both.
     */
    if (sc->fixed) {
	AcpiClearEvent(event);
	status = AcpiInstallFixedEventHandler(event,
			acpi_button_fixed_handler, sc);
    } else {
	/*
	 * If a system does not get lid events, it may make sense to change
	 * the type to ACPI_ALL_NOTIFY.  Some systems generate both a wake
	 * and runtime notify in that case though.
	 */
	status = AcpiInstallNotifyHandler(sc->button_handle,
			ACPI_DEVICE_NOTIFY, acpi_button_notify_handler, sc);
    }
    if (ACPI_FAILURE(status)) {
	device_printf(sc->button_dev, "couldn't install notify handler - %s\n",
		      AcpiFormatException(status));
	return_VALUE (ENXIO);
    }

    /* Enable the GPE for wake/runtime. */
    acpi_wake_set_enable(dev, 1);
    if (acpi_parse_prw(sc->button_handle, &prw) == 0)
	AcpiEnableGpe(prw.gpe_handle, prw.gpe_bit);

    return_VALUE (0);
}

static int
acpi_button_suspend(device_t dev)
{
    return (0);
}

static int
acpi_button_resume(device_t dev)
{
    return (0);
}

static void
acpi_button_notify_sleep(void *arg)
{
    struct acpi_button_softc	*sc;
    struct acpi_softc		*acpi_sc;

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    sc = (struct acpi_button_softc *)arg;
    acpi_sc = acpi_device_get_parent_softc(sc->button_dev);
    if (acpi_sc == NULL)
	return_VOID;

    acpi_UserNotify("Button", sc->button_handle, sc->button_type);

    switch (sc->button_type) {
    case ACPI_POWER_BUTTON:
	ACPI_VPRINT(sc->button_dev, acpi_sc, "power button pressed\n");
	acpi_event_power_button_sleep(acpi_sc);
	break;
    case ACPI_SLEEP_BUTTON:
	ACPI_VPRINT(sc->button_dev, acpi_sc, "sleep button pressed\n");
	acpi_event_sleep_button_sleep(acpi_sc);
	break;
    default:
	break;		/* unknown button type */
    }
}

static void
acpi_button_notify_wakeup(void *arg)
{
    struct acpi_button_softc	*sc;
    struct acpi_softc		*acpi_sc;

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    sc = (struct acpi_button_softc *)arg;
    acpi_sc = acpi_device_get_parent_softc(sc->button_dev);
    if (acpi_sc == NULL)
	return_VOID;

    acpi_UserNotify("Button", sc->button_handle, sc->button_type);

    switch (sc->button_type) {
    case ACPI_POWER_BUTTON:
	ACPI_VPRINT(sc->button_dev, acpi_sc, "wakeup by power button\n");
	acpi_event_power_button_wake(acpi_sc);
	break;
    case ACPI_SLEEP_BUTTON:
	ACPI_VPRINT(sc->button_dev, acpi_sc, "wakeup by sleep button\n");
	acpi_event_sleep_button_wake(acpi_sc);
	break;
    default:
	break;		/* unknown button type */
    }
}

static void 
acpi_button_notify_handler(ACPI_HANDLE h, UINT32 notify, void *context)
{
    struct acpi_button_softc	*sc;

    ACPI_FUNCTION_TRACE_U32((char *)(uintptr_t)__func__, notify);

    sc = (struct acpi_button_softc *)context;
    switch (notify) {
    case ACPI_NOTIFY_BUTTON_PRESSED_FOR_SLEEP:
	AcpiOsExecute(OSL_NOTIFY_HANDLER, acpi_button_notify_sleep, sc);
	break;   
    case ACPI_NOTIFY_BUTTON_PRESSED_FOR_WAKEUP:
	AcpiOsExecute(OSL_NOTIFY_HANDLER, acpi_button_notify_wakeup, sc);
	break;   
    default:
	device_printf(sc->button_dev, "unknown notify %#x\n", notify);
	break;
    }
}

static ACPI_STATUS 
acpi_button_fixed_handler(void *context)
{
    struct acpi_button_softc	*sc = (struct acpi_button_softc *)context;

    ACPI_FUNCTION_TRACE_PTR((char *)(uintptr_t)__func__, context);

    if (context == NULL)
	return_ACPI_STATUS (AE_BAD_PARAMETER);

    acpi_button_notify_handler(sc->button_handle,
			       ACPI_NOTIFY_BUTTON_PRESSED_FOR_SLEEP, sc);
    return_ACPI_STATUS (AE_OK);
}