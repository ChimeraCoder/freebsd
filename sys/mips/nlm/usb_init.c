
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
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/kernel.h>

#include <mips/nlm/hal/haldefs.h>
#include <mips/nlm/hal/iomap.h>
#include <mips/nlm/hal/usb.h>

#include <mips/nlm/xlp.h>


static void
nlm_usb_intr_en(int node, int port)
{
	uint32_t val;
	uint64_t port_addr;

	port_addr = nlm_get_usb_regbase(node, port);	
	val = nlm_read_usb_reg(port_addr, USB_INT_EN);
	val = USB_CTRL_INTERRUPT_EN  | USB_OHCI_INTERRUPT_EN | 
		USB_OHCI_INTERRUPT1_EN | USB_CTRL_INTERRUPT_EN  |
		USB_OHCI_INTERRUPT_EN | USB_OHCI_INTERRUPT2_EN;
        nlm_write_usb_reg(port_addr, USB_INT_EN, val);
}

static void 
nlm_usb_hw_reset(int node, int port)
{
	uint64_t port_addr;
	uint32_t val;
        	
	/* reset USB phy */
	port_addr = nlm_get_usb_regbase(node, port); 
	val = nlm_read_usb_reg(port_addr, USB_PHY_0);
	val &= ~(USB_PHY_RESET | USB_PHY_PORT_RESET_0 | USB_PHY_PORT_RESET_1);
	nlm_write_usb_reg(port_addr, USB_PHY_0, val);
      
	DELAY(100);
	val = nlm_read_usb_reg(port_addr, USB_CTL_0);
	val &= ~(USB_CONTROLLER_RESET);
	val |= 0x4;
	nlm_write_usb_reg(port_addr, USB_CTL_0, val);
}

static void 
nlm_usb_init(void)
{
	/* XXX: should be checking if these are in Device mode here */
	printf("Initialize USB Interface\n");
	nlm_usb_hw_reset(0, 0); 
	nlm_usb_hw_reset(0, 3); 

	/* Enable PHY interrupts */
	nlm_usb_intr_en(0, 0); 
	nlm_usb_intr_en(0, 3); 
}

SYSINIT(nlm_usb_init, SI_SUB_CPU, SI_ORDER_MIDDLE,
    nlm_usb_init, NULL);