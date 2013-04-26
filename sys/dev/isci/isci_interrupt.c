
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

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>

#include <dev/isci/scil/scif_controller.h>

void isci_interrupt_legacy_handler(void *arg);
void isci_interrupt_msix_handler(void *arg);

static int
isci_interrupt_setup_legacy(struct isci_softc *isci)
{
	struct ISCI_INTERRUPT_INFO *interrupt_info = &isci->interrupt_info[0];

	isci->num_interrupts = 1;

	scic_controller_get_handler_methods(SCIC_LEGACY_LINE_INTERRUPT_TYPE,
	    0, &isci->handlers[0]);

	interrupt_info->handlers = &isci->handlers[0];
	interrupt_info->rid = 0;
	interrupt_info->interrupt_target_handle = (void *)isci;

	interrupt_info->res = bus_alloc_resource_any(isci->device, SYS_RES_IRQ,
	    &interrupt_info->rid, RF_SHAREABLE|RF_ACTIVE);

	if (interrupt_info->res == NULL) {
		isci_log_message(0, "ISCI", "bus_alloc_resource failed\n");
		return (-1);
	}

	interrupt_info->tag = NULL;
	if (bus_setup_intr(isci->device, interrupt_info->res,
	    INTR_TYPE_CAM | INTR_MPSAFE, NULL, isci_interrupt_legacy_handler,
	    interrupt_info, &interrupt_info->tag)) {
		isci_log_message(0, "ISCI", "bus_setup_intr failed\n");
		return (-1);
	}

	return (0);
}

static int
isci_interrupt_setup_msix(struct isci_softc *isci)
{
	uint32_t controller_index;

	scic_controller_get_handler_methods(SCIC_MSIX_INTERRUPT_TYPE,
	    SCI_MAX_MSIX_MESSAGES_PER_CONTROLLER, &isci->handlers[0]);

	for (controller_index = 0; controller_index < isci->controller_count;
	    controller_index++) {
		uint32_t msix_index;
		uint8_t base_index = controller_index *
		    SCI_MAX_MSIX_MESSAGES_PER_CONTROLLER;

		for (msix_index = 0; msix_index < SCI_MAX_MSIX_MESSAGES_PER_CONTROLLER;
		    msix_index++) {
			struct ISCI_INTERRUPT_INFO *info =
			    &isci->interrupt_info[base_index+msix_index];

			info->handlers = &isci->handlers[msix_index];
			info->interrupt_target_handle =
			    &isci->controllers[controller_index];

			info->rid = base_index+msix_index+1;

			info->res = bus_alloc_resource_any(isci->device,
			    SYS_RES_IRQ, &info->rid, RF_ACTIVE);
			if (info->res == NULL) {
				isci_log_message(0, "ISCI",
				    "bus_alloc_resource failed\n");
				return (-1);
			}

			info->tag = NULL;
			if (bus_setup_intr(isci->device, info->res,
			    INTR_TYPE_CAM | INTR_MPSAFE, NULL,
			    isci_interrupt_msix_handler, info, &info->tag)) {
				isci_log_message(0, "ISCI",
				    "bus_setup_intr failed\n");
				return (-1);
			}
		}
	}

	return (0);
}

void
isci_interrupt_setup(struct isci_softc *isci)
{
	uint8_t max_msix_messages = SCI_MAX_MSIX_MESSAGES_PER_CONTROLLER *
	    isci->controller_count;
	BOOL use_msix = FALSE;
	uint32_t force_legacy_interrupts = 0;

	TUNABLE_INT_FETCH("hw.isci.force_legacy_interrupts",
	    &force_legacy_interrupts);

	if (!force_legacy_interrupts &&
	    pci_msix_count(isci->device) >= max_msix_messages) {

		isci->num_interrupts = max_msix_messages;
		pci_alloc_msix(isci->device, &isci->num_interrupts);
		if (isci->num_interrupts == max_msix_messages)
			use_msix = TRUE;
	}

	if (use_msix == TRUE)
		isci_interrupt_setup_msix(isci);
	else
		isci_interrupt_setup_legacy(isci);
}

void
isci_interrupt_legacy_handler(void *arg)
{
	struct ISCI_INTERRUPT_INFO *interrupt_info =
	    (struct ISCI_INTERRUPT_INFO *)arg;
	struct isci_softc *isci =
	    (struct isci_softc *)interrupt_info->interrupt_target_handle;
	SCIC_CONTROLLER_INTERRUPT_HANDLER  interrupt_handler;
	SCIC_CONTROLLER_COMPLETION_HANDLER completion_handler;
	int index;

	interrupt_handler =  interrupt_info->handlers->interrupt_handler;
	completion_handler = interrupt_info->handlers->completion_handler;

	for (index = 0; index < isci->controller_count; index++) {
		struct ISCI_CONTROLLER *controller = &isci->controllers[index];

		/* If controller_count > 0, we will get interrupts here for
		 *  controller 0 before controller 1 has even started.  So
		 *  we need to make sure we don't call the completion handler
		 *  for a non-started controller.
		 */
		if (controller->is_started == TRUE) {
			SCI_CONTROLLER_HANDLE_T scic_controller_handle =
			    scif_controller_get_scic_handle(
				controller->scif_controller_handle);

			if (interrupt_handler(scic_controller_handle)) {
				mtx_lock(&controller->lock);
				completion_handler(scic_controller_handle);
				if (controller->release_queued_ccbs == TRUE)
					isci_controller_release_queued_ccbs(
					    controller);
				mtx_unlock(&controller->lock);
			}
		}
	}
}

void
isci_interrupt_msix_handler(void *arg)
{
	struct ISCI_INTERRUPT_INFO *interrupt_info =
	    (struct ISCI_INTERRUPT_INFO *)arg;
	struct ISCI_CONTROLLER *controller =
	    (struct ISCI_CONTROLLER *)interrupt_info->interrupt_target_handle;
	SCIC_CONTROLLER_INTERRUPT_HANDLER  interrupt_handler;
	SCIC_CONTROLLER_COMPLETION_HANDLER completion_handler;

	interrupt_handler =  interrupt_info->handlers->interrupt_handler;
	completion_handler = interrupt_info->handlers->completion_handler;

	SCI_CONTROLLER_HANDLE_T scic_controller_handle;

	scic_controller_handle = scif_controller_get_scic_handle(
	    controller->scif_controller_handle);

	if (interrupt_handler(scic_controller_handle)) {
		mtx_lock(&controller->lock);
		completion_handler(scic_controller_handle);
		/*
		 * isci_controller_release_queued_ccb() is a relatively
		 *  expensive routine, so we don't call it until the controller
		 *  level flag is set to TRUE.
		 */
		if (controller->release_queued_ccbs == TRUE)
			isci_controller_release_queued_ccbs(controller);
		mtx_unlock(&controller->lock);
	}
}

void
isci_interrupt_poll_handler(struct ISCI_CONTROLLER *controller)
{
	SCI_CONTROLLER_HANDLE_T scic_controller =
	    scif_controller_get_scic_handle(controller->scif_controller_handle);
	SCIC_CONTROLLER_HANDLER_METHODS_T handlers;

	scic_controller_get_handler_methods(SCIC_NO_INTERRUPTS, 0x0, &handlers);

	if(handlers.interrupt_handler(scic_controller) == TRUE) {
		/* Do not acquire controller lock in this path. xpt
		 *  poll routine will get called with this lock already
		 *  held, so we can't acquire it again here.  Other users
		 *  of this function must acquire the lock explicitly
		 *  before calling this handler.
		 */
		handlers.completion_handler(scic_controller);
	}
}