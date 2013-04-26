
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

#include <dev/isci/isci.h>

#include <sys/sysctl.h>

#include <dev/isci/scil/scif_controller.h>
#include <dev/isci/scil/scic_phy.h>

static int
isci_sysctl_coalesce_timeout(SYSCTL_HANDLER_ARGS)
{
	struct isci_softc *isci = (struct isci_softc *)arg1;
	int error = sysctl_handle_int(oidp, &isci->coalesce_timeout, 0, req);
	int i;

	if (error)
		return (error);

	for (i = 0; i < isci->controller_count; i++)
		scif_controller_set_interrupt_coalescence(
		    isci->controllers[i].scif_controller_handle,
		    isci->coalesce_number, isci->coalesce_timeout);

	return (0);
}

static int
isci_sysctl_coalesce_number(SYSCTL_HANDLER_ARGS)
{
	struct isci_softc *isci = (struct isci_softc *)arg1;
	int error = sysctl_handle_int(oidp, &isci->coalesce_number, 0, req);
	int i;

	if (error)
		return (error);

	for (i = 0; i < isci->controller_count; i++)
		scif_controller_set_interrupt_coalescence(
		    isci->controllers[i].scif_controller_handle,
		    isci->coalesce_number, isci->coalesce_timeout);

	return (0);
}

static void
isci_sysctl_reset_remote_devices(struct ISCI_CONTROLLER *controller,
    uint32_t remote_devices_to_be_reset)
{
	uint32_t i = 0;

	while (remote_devices_to_be_reset != 0) {
		if (remote_devices_to_be_reset & 0x1) {
			struct ISCI_REMOTE_DEVICE *remote_device =
				controller->remote_device[i];

			if (remote_device != NULL) {
				mtx_lock(&controller->lock);
				isci_remote_device_reset(remote_device, NULL);
				mtx_unlock(&controller->lock);
			}
		}
		remote_devices_to_be_reset >>= 1;
		i++;
	}
}

static int
isci_sysctl_reset_remote_device_on_controller0(SYSCTL_HANDLER_ARGS)
{
	struct isci_softc *isci = (struct isci_softc *)arg1;
	uint32_t remote_devices_to_be_reset = 0;
	struct ISCI_CONTROLLER *controller = &isci->controllers[0];
	int error = sysctl_handle_int(oidp, &remote_devices_to_be_reset, 0, req);

	if (error || remote_devices_to_be_reset == 0)
		return (error);

	isci_sysctl_reset_remote_devices(controller, remote_devices_to_be_reset);

	return (0);
}

static int
isci_sysctl_reset_remote_device_on_controller1(SYSCTL_HANDLER_ARGS)
{
	struct isci_softc *isci = (struct isci_softc *)arg1;
	uint32_t remote_devices_to_be_reset = 0;
	struct ISCI_CONTROLLER *controller = &isci->controllers[1];
	int error =
	    sysctl_handle_int(oidp, &remote_devices_to_be_reset, 0, req);

	if (error || remote_devices_to_be_reset == 0)
		return (error);

	isci_sysctl_reset_remote_devices(controller,
	    remote_devices_to_be_reset);

	return (0);
}

static void
isci_sysctl_stop(struct ISCI_CONTROLLER *controller, uint32_t phy_to_be_stopped)
{
	SCI_PHY_HANDLE_T phy_handle = NULL;

	scic_controller_get_phy_handle(
	    scif_controller_get_scic_handle(controller->scif_controller_handle),
	    phy_to_be_stopped, &phy_handle);

	scic_phy_stop(phy_handle);
}

static int
isci_sysctl_stop_phy(SYSCTL_HANDLER_ARGS)
{
	struct isci_softc *isci = (struct isci_softc *)arg1;
	uint32_t phy_to_be_stopped = 0xff;
	uint32_t controller_index, phy_index;
	int error = sysctl_handle_int(oidp, &phy_to_be_stopped, 0, req);

	controller_index = phy_to_be_stopped / SCI_MAX_PHYS;
	phy_index = phy_to_be_stopped % SCI_MAX_PHYS;

	if(error || controller_index >= isci->controller_count)
		return (error);

	isci_sysctl_stop(&isci->controllers[controller_index], phy_index);

	return (0);
}

static void
isci_sysctl_start(struct ISCI_CONTROLLER *controller,
    uint32_t phy_to_be_started)
{
	SCI_PHY_HANDLE_T phy_handle = NULL;

	scic_controller_get_phy_handle(
	    scif_controller_get_scic_handle(controller->scif_controller_handle),
	    phy_to_be_started, &phy_handle);

	scic_phy_start(phy_handle);
}

static int
isci_sysctl_start_phy(SYSCTL_HANDLER_ARGS)
{
	struct isci_softc *isci = (struct isci_softc *)arg1;
	uint32_t phy_to_be_started = 0xff;
	uint32_t controller_index, phy_index;
	int error = sysctl_handle_int(oidp, &phy_to_be_started, 0, req);

	controller_index = phy_to_be_started / SCI_MAX_PHYS;
	phy_index = phy_to_be_started % SCI_MAX_PHYS;

	if(error || controller_index >= isci->controller_count)
		return error;

	isci_sysctl_start(&isci->controllers[controller_index], phy_index);

	return 0;
}

void isci_sysctl_initialize(struct isci_softc *isci)
{
	struct sysctl_ctx_list *sysctl_ctx = device_get_sysctl_ctx(isci->device);
	struct sysctl_oid *sysctl_tree = device_get_sysctl_tree(isci->device);

	SYSCTL_ADD_PROC(sysctl_ctx, SYSCTL_CHILDREN(sysctl_tree), OID_AUTO,
	    "coalesce_timeout", CTLTYPE_UINT | CTLFLAG_RW, isci, 0,
	    isci_sysctl_coalesce_timeout, "IU",
	    "Interrupt coalescing timeout (in microseconds)");

	SYSCTL_ADD_PROC(sysctl_ctx, SYSCTL_CHILDREN(sysctl_tree), OID_AUTO,
	    "coalesce_number", CTLTYPE_UINT | CTLFLAG_RW, isci, 0,
	    isci_sysctl_coalesce_number, "IU",
	    "Interrupt coalescing number");

	SYSCTL_ADD_PROC(sysctl_ctx, SYSCTL_CHILDREN(sysctl_tree), OID_AUTO,
	    "reset_remote_device_on_controller0", CTLTYPE_UINT| CTLFLAG_RW,
	    isci, 0, isci_sysctl_reset_remote_device_on_controller0, "IU",
	    "Reset remote device on controller 0");

	SYSCTL_ADD_PROC(sysctl_ctx, SYSCTL_CHILDREN(sysctl_tree), OID_AUTO,
	    "reset_remote_device_on_controller1", CTLTYPE_UINT| CTLFLAG_RW,
	    isci, 0, isci_sysctl_reset_remote_device_on_controller1, "IU",
	    "Reset remote device on controller 1");

	SYSCTL_ADD_PROC(sysctl_ctx, SYSCTL_CHILDREN(sysctl_tree), OID_AUTO,
	    "stop_phy", CTLTYPE_UINT| CTLFLAG_RW, isci, 0, isci_sysctl_stop_phy,
	    "IU", "Stop PHY on a controller");

	SYSCTL_ADD_PROC(sysctl_ctx, SYSCTL_CHILDREN(sysctl_tree), OID_AUTO,
	    "start_phy", CTLTYPE_UINT| CTLFLAG_RW, isci, 0,
	    isci_sysctl_start_phy, "IU", "Start PHY on a controller");
}