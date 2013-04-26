
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

#include <sys/types.h>
#include <sys/endian.h>
#include <errno.h>
#include <netgraph/bluetooth/include/ng_hci.h>
#include <stdio.h>
#include "hccontrol.h"

/* Send Read_Failed_Contact_Counter command to the unit */
static int
hci_read_failed_contact_counter(int s, int argc, char **argv)
{
	ng_hci_read_failed_contact_cntr_cp	cp;
	ng_hci_read_failed_contact_cntr_rp	rp;
	int					n;

	switch (argc) {
	case 1:
		/* connection handle */
		if (sscanf(argv[0], "%d", &n) != 1 || n <= 0 || n > 0x0eff)
			return (USAGE);
  
		cp.con_handle = (uint16_t) (n & 0x0fff);
		cp.con_handle = htole16(cp.con_handle);
		break;

	default:
		return (USAGE);
	} 

	/* send command */
	n = sizeof(rp);
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_STATUS,
			NG_HCI_OCF_READ_FAILED_CONTACT_CNTR),
			(char const *) &cp, sizeof(cp),
			(char *) &rp, &n) == ERROR)
		return (ERROR);

	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	fprintf(stdout, "Connection handle: %d\n", le16toh(rp.con_handle));
	fprintf(stdout, "Failed contact counter: %d\n", le16toh(rp.counter));

	return (OK);
} /* hci_read_failed_contact_counter */

/* Send Reset_Failed_Contact_Counter command to the unit */
static int
hci_reset_failed_contact_counter(int s, int argc, char **argv)
{
	ng_hci_reset_failed_contact_cntr_cp	cp;
	ng_hci_reset_failed_contact_cntr_rp	rp;
	int					n;

	switch (argc) {
	case 1:
		/* connection handle */
		if (sscanf(argv[0], "%d", &n) != 1 || n <= 0 || n > 0x0eff)
			return (USAGE);
  
		cp.con_handle = (uint16_t) (n & 0x0fff);
		cp.con_handle = htole16(cp.con_handle);
		break;

	default:
		return (USAGE);
	}

	/* send command */
	n = sizeof(rp);
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_STATUS,
			NG_HCI_OCF_RESET_FAILED_CONTACT_CNTR),
			(char const *) &cp, sizeof(cp),
			(char *) &rp, &n) == ERROR)
		return (ERROR);

	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}
	
	return (OK);
} /* hci_reset_failed_contact_counter */

/* Sent Get_Link_Quality command to the unit */
static int
hci_get_link_quality(int s, int argc, char **argv)
{
	ng_hci_get_link_quality_cp	cp;
	ng_hci_get_link_quality_rp	rp;
	int				n;

	switch (argc) {
	case 1:
		/* connection handle */
		if (sscanf(argv[0], "%d", &n) != 1 || n <= 0 || n > 0x0eff)
			return (USAGE);
  
		cp.con_handle = (uint16_t) (n & 0x0fff);
		cp.con_handle = htole16(cp.con_handle);
		break;

	default:
		return (USAGE);
	}

	/* send command */
	n = sizeof(rp);
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_STATUS,
			NG_HCI_OCF_GET_LINK_QUALITY),
			(char const *) &cp, sizeof(cp),
			(char *) &rp, &n) == ERROR)
		return (ERROR);

	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	fprintf(stdout, "Connection handle: %d\n", le16toh(rp.con_handle));
	fprintf(stdout, "Link quality: %d\n", le16toh(rp.quality));
	
	return (OK);
} /* hci_get_link_quality */

/* Send Read_RSSI command to the unit */
static int
hci_read_rssi(int s, int argc, char **argv)
{
	ng_hci_read_rssi_cp	cp;
	ng_hci_read_rssi_rp	rp;
	int			n;
	
	switch (argc) {
	case 1:
		/* connection handle */
		if (sscanf(argv[0], "%d", &n) != 1 || n <= 0 || n > 0x0eff)
			return (USAGE);
  
		cp.con_handle = (uint16_t) (n & 0x0fff);
		cp.con_handle = htole16(cp.con_handle);
		break;

	default:
		return (USAGE);
	}

	/* send command */
	n = sizeof(rp);
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_STATUS,
			NG_HCI_OCF_READ_RSSI),
			(char const *) &cp, sizeof(cp),
			(char *) &rp, &n) == ERROR)
		return (ERROR);

	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	fprintf(stdout, "Connection handle: %d\n", le16toh(rp.con_handle));
	fprintf(stdout, "RSSI: %d dB\n", (int) rp.rssi);
	
	return (OK);
} /* hci_read_rssi */

struct hci_command	status_commands[] = {
{
"read_failed_contact_counter <connection_handle>",
"\nThis command will read the value for the Failed_Contact_Counter\n" \
"parameter for a particular ACL connection to another device.\n\n" \
"\t<connection_handle> - dddd; ACL connection handle\n",
&hci_read_failed_contact_counter
},
{
"reset_failed_contact_counter <connection_handle>",
"\nThis command will reset the value for the Failed_Contact_Counter\n" \
"parameter for a particular ACL connection to another device.\n\n" \
"\t<connection_handle> - dddd; ACL connection handle\n",
&hci_reset_failed_contact_counter
},
{
"get_link_quality <connection_handle>",
"\nThis command will return the value for the Link_Quality for the\n" \
"specified ACL connection handle. This command will return a Link_Quality\n" \
"value from 0-255, which represents the quality of the link between two\n" \
"Bluetooth devices. The higher the value, the better the link quality is.\n" \
"Each Bluetooth module vendor will determine how to measure the link quality." \
"\n\n" \
"\t<connection_handle> - dddd; ACL connection handle\n", 
&hci_get_link_quality
},
{
"read_rssi <connection_handle>",
"\nThis command will read the value for the difference between the\n" \
"measured Received Signal Strength Indication (RSSI) and the limits of\n" \
"the Golden Receive Power Range for a ACL connection handle to another\n" \
"Bluetooth device. Any positive RSSI value returned by the Host Controller\n" \
"indicates how many dB the RSSI is above the upper limit, any negative\n" \
"value indicates how many dB the RSSI is below the lower limit. The value\n" \
"zero indicates that the RSSI is inside the Golden Receive Power Range.\n\n" \
"\t<connection_handle> - dddd; ACL connection handle\n", 
&hci_read_rssi
},
{
NULL,
}};