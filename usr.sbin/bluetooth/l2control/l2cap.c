
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

#include <sys/ioctl.h>
#include <bluetooth.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "l2control.h"

#define	SIZE(x)	(sizeof((x))/sizeof((x)[0]))

/* Print BDADDR */
static char *
bdaddrpr(bdaddr_t const *ba)
{
	extern int	 numeric_bdaddr;
	static char	 str[24];
	struct hostent	*he = NULL;

	if (memcmp(ba, NG_HCI_BDADDR_ANY, sizeof(*ba)) == 0) {
		str[0] = '*';
		str[1] = 0;

		return (str);
	}

	if (!numeric_bdaddr &&
	    (he = bt_gethostbyaddr((char *)ba, sizeof(*ba), AF_BLUETOOTH)) != NULL) {
		strlcpy(str, he->h_name, sizeof(str));

		return (str);
	}

	bt_ntoa(ba, str);

	return (str);
} /* bdaddrpr */

/* Send read_node_flags command to the node */
static int
l2cap_read_node_flags(int s, int argc, char **argv)
{
	struct ng_btsocket_l2cap_raw_node_flags	r;

	memset(&r, 0, sizeof(r));
	if (ioctl(s, SIOC_L2CAP_NODE_GET_FLAGS, &r, sizeof(r)) < 0)
		return (ERROR);

	fprintf(stdout, "Connectionless traffic flags:\n");
	fprintf(stdout, "\tSDP: %s\n",
		(r.flags & NG_L2CAP_CLT_SDP_DISABLED)? "disabled" : "enabled");
	fprintf(stdout, "\tRFCOMM: %s\n",
		(r.flags & NG_L2CAP_CLT_RFCOMM_DISABLED)? "disabled":"enabled");
	fprintf(stdout, "\tTCP: %s\n",
		(r.flags & NG_L2CAP_CLT_TCP_DISABLED)? "disabled" : "enabled");

	return (OK);
} /* l2cap_read_node_flags */

/* Send read_debug_level command to the node */
static int
l2cap_read_debug_level(int s, int argc, char **argv)
{
	struct ng_btsocket_l2cap_raw_node_debug	r;

	memset(&r, 0, sizeof(r));
	if (ioctl(s, SIOC_L2CAP_NODE_GET_DEBUG, &r, sizeof(r)) < 0)
		return (ERROR);

	fprintf(stdout, "Debug level: %d\n", r.debug);

	return (OK);
} /* l2cap_read_debug_level */

/* Send write_debug_level command to the node */
static int
l2cap_write_debug_level(int s, int argc, char **argv)
{
	struct ng_btsocket_l2cap_raw_node_debug	r;

	memset(&r, 0, sizeof(r));
	switch (argc) {
	case 1:
		r.debug = atoi(argv[0]);
		break;

	default:
		return (USAGE);
	}

	if (ioctl(s, SIOC_L2CAP_NODE_SET_DEBUG, &r, sizeof(r)) < 0)
		return (ERROR);

	return (OK);
} /* l2cap_write_debug_level */

/* Send read_connection_list command to the node */
static int
l2cap_read_connection_list(int s, int argc, char **argv)
{
	static char const * const	state[] = {
		/* NG_L2CAP_CON_CLOSED */	"CLOSED",
		/* NG_L2CAP_W4_LP_CON_CFM */	"W4_LP_CON_CFM",
		/* NG_L2CAP_CON_OPEN */		"OPEN"
	};
#define con_state2str(x)	((x) >= SIZE(state)? "UNKNOWN" : state[(x)])

	struct ng_btsocket_l2cap_raw_con_list	r;
	int					n, error = OK;

	memset(&r, 0, sizeof(r));
	r.num_connections = NG_L2CAP_MAX_CON_NUM;
	r.connections = calloc(NG_L2CAP_MAX_CON_NUM,
				sizeof(ng_l2cap_node_con_ep));
	if (r.connections == NULL) {
		errno = ENOMEM;
		return (ERROR);
	}

	if (ioctl(s, SIOC_L2CAP_NODE_GET_CON_LIST, &r, sizeof(r)) < 0) {
		error = ERROR;
		goto out;
	}

	fprintf(stdout, "L2CAP connections:\n");
	fprintf(stdout, 
"Remote BD_ADDR    Handle Flags Pending State\n");
	for (n = 0; n < r.num_connections; n++) {
		fprintf(stdout,
			"%-17.17s " \
			"%6d " \
			"%c%c%c%c%c " \
			"%7d " \
			"%s\n",
			bdaddrpr(&r.connections[n].remote),
			r.connections[n].con_handle, 
			((r.connections[n].flags & NG_L2CAP_CON_OUTGOING)? 'O' : 'I'),
			((r.connections[n].flags & NG_L2CAP_CON_LP_TIMO)? 'L' : ' '),
			((r.connections[n].flags & NG_L2CAP_CON_AUTO_DISCON_TIMO)? 'D' : ' '),
			((r.connections[n].flags & NG_L2CAP_CON_TX)? 'T' : ' '),
			((r.connections[n].flags & NG_L2CAP_CON_RX)? 'R' : ' '),
			r.connections[n].pending,
			con_state2str(r.connections[n].state));
	}
out:
	free(r.connections);

	return (error);
} /* l2cap_read_connection_list */

/* Send read_channel_list command to the node */
static int
l2cap_read_channel_list(int s, int argc, char **argv)
{
	static char const * const	state[] = {
		/* NG_L2CAP_CLOSED */			"CLOSED",
		/* NG_L2CAP_W4_L2CAP_CON_RSP */		"W4_L2CAP_CON_RSP",
		/* NG_L2CAP_W4_L2CA_CON_RSP */		"W4_L2CA_CON_RSP",
		/* NG_L2CAP_CONFIG */			"CONFIG",
		/* NG_L2CAP_OPEN */			"OPEN",
		/* NG_L2CAP_W4_L2CAP_DISCON_RSP */	"W4_L2CAP_DISCON_RSP",
		/* NG_L2CAP_W4_L2CA_DISCON_RSP */	"W4_L2CA_DISCON_RSP"
	};
#define ch_state2str(x)	((x) >= SIZE(state)? "UNKNOWN" : state[(x)])

	struct ng_btsocket_l2cap_raw_chan_list	r;
	int					n, error = OK;

	memset(&r, 0, sizeof(r));
	r.num_channels = NG_L2CAP_MAX_CHAN_NUM;
	r.channels = calloc(NG_L2CAP_MAX_CHAN_NUM,
				sizeof(ng_l2cap_node_chan_ep));
	if (r.channels == NULL) {
		errno = ENOMEM;
		return (ERROR);
	}

	if (ioctl(s, SIOC_L2CAP_NODE_GET_CHAN_LIST, &r, sizeof(r)) < 0) {
		error = ERROR;
		goto out;
	}

	fprintf(stdout, "L2CAP channels:\n");
	fprintf(stdout, 
"Remote BD_ADDR     SCID/ DCID   PSM  IMTU/ OMTU State\n");
	for (n = 0; n < r.num_channels; n++) {
		fprintf(stdout,
			"%-17.17s " \
			"%5d/%5d %5d " \
			"%5d/%5d " \
			"%s\n",
			bdaddrpr(&r.channels[n].remote),
			r.channels[n].scid, r.channels[n].dcid,
			r.channels[n].psm, r.channels[n].imtu,
			r.channels[n].omtu,
			ch_state2str(r.channels[n].state));
	}
out:
	free(r.channels);

	return (error);
} /* l2cap_read_channel_list */

/* Send read_auto_disconnect_timeout command to the node */
static int
l2cap_read_auto_disconnect_timeout(int s, int argc, char **argv)
{
	struct ng_btsocket_l2cap_raw_auto_discon_timo	r;

	memset(&r, 0, sizeof(r));
	if (ioctl(s, SIOC_L2CAP_NODE_GET_AUTO_DISCON_TIMO, &r, sizeof(r)) < 0)
		return (ERROR);

	if (r.timeout != 0)
		fprintf(stdout, "Auto disconnect timeout: %d sec\n", r.timeout);
	else
		fprintf(stdout, "Auto disconnect disabled\n");

	return (OK);
} /* l2cap_read_auto_disconnect_timeout */

/* Send write_auto_disconnect_timeout command to the node */
static int
l2cap_write_auto_disconnect_timeout(int s, int argc, char **argv)
{
	struct ng_btsocket_l2cap_raw_auto_discon_timo	r;

	memset(&r, 0, sizeof(r));
	switch (argc) {
	case 1:
		r.timeout = atoi(argv[0]);
		break;

	default:
		return (USAGE);
	}

	if (ioctl(s, SIOC_L2CAP_NODE_SET_AUTO_DISCON_TIMO, &r, sizeof(r)) < 0)
		return (ERROR);

	return (OK);
} /* l2cap_write_auto_disconnect_timeout */

struct l2cap_command	l2cap_commands[] = {
{
"read_node_flags",
"Get L2CAP node flags",
&l2cap_read_node_flags
},
{
"read_debug_level",
"Get L2CAP node debug level",
&l2cap_read_debug_level
},
{
"write_debug_level <level>",
"Set L2CAP node debug level",
&l2cap_write_debug_level
},
{
"read_connection_list",
"Read list of the L2CAP connections",
&l2cap_read_connection_list
},
{
"read_channel_list",
"Read list of the L2CAP channels",
&l2cap_read_channel_list
},
{
"read_auto_disconnect_timeout",
"Get L2CAP node auto disconnect timeout (in sec)",
&l2cap_read_auto_disconnect_timeout
},
{
"write_auto_disconnect_timeout <timeout>",
"Set L2CAP node auto disconnect timeout (in sec)",
&l2cap_write_auto_disconnect_timeout
},
{
NULL,
}};