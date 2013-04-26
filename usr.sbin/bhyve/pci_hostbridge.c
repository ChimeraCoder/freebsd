
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

#include "pci_emul.h"

static int
pci_hostbridge_init(struct vmctx *ctx, struct pci_devinst *pi, char *opts)
{

	/* config space */
	pci_set_cfgdata16(pi, PCIR_VENDOR, 0x1275);	/* NetApp */
	pci_set_cfgdata16(pi, PCIR_DEVICE, 0x1275);	/* NetApp */
	pci_set_cfgdata8(pi, PCIR_HDRTYPE, PCIM_HDRTYPE_BRIDGE);
	pci_set_cfgdata8(pi, PCIR_CLASS, PCIC_BRIDGE);
	pci_set_cfgdata8(pi, PCIR_SUBCLASS, PCIS_BRIDGE_HOST);

	pci_emul_add_pciecap(pi, PCIEM_TYPE_ROOT_PORT);

	return (0);
}

struct pci_devemu pci_de_hostbridge = {
	.pe_emu = "hostbridge",
	.pe_init = pci_hostbridge_init,
};
PCI_EMUL_SET(pci_de_hostbridge);