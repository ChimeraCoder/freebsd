
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

/*
 * Abstract:
 *    Implementation of OpenSM unicast routing module which loads
 *    routes from the dump file
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <iba/ib_types.h>
#include <complib/cl_qmap.h>
#include <complib/cl_debug.h>
#include <opensm/osm_opensm.h>
#include <opensm/osm_switch.h>
#include <opensm/osm_log.h>

static uint16_t remap_lid(osm_opensm_t * p_osm, uint16_t lid, ib_net64_t guid)
{
	osm_port_t *p_port;
	uint16_t min_lid, max_lid;
	uint8_t lmc;

	p_port = osm_get_port_by_guid(&p_osm->subn, guid);
	if (!p_port) {
		OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
			"cannot find port guid 0x%016" PRIx64
			" , will use the same lid\n", cl_ntoh64(guid));
		return lid;
	}

	osm_port_get_lid_range_ho(p_port, &min_lid, &max_lid);
	if (min_lid <= lid && lid <= max_lid)
		return lid;

	lmc = osm_port_get_lmc(p_port);
	return min_lid + (lid & ((1 << lmc) - 1));
}

static void add_path(osm_opensm_t * p_osm,
		     osm_switch_t * p_sw, uint16_t lid, uint8_t port_num,
		     ib_net64_t port_guid)
{
	uint16_t new_lid;
	uint8_t old_port;

	new_lid = port_guid ? remap_lid(p_osm, lid, port_guid) : lid;
	old_port = osm_switch_get_port_by_lid(p_sw, new_lid);
	if (old_port != OSM_NO_PATH && old_port != port_num) {
		OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
			"LID collision is detected on switch "
			"0x016%" PRIx64 ", will overwrite LID %u entry\n",
			cl_ntoh64(osm_node_get_node_guid(p_sw->p_node)),
			new_lid);
	}

	p_sw->new_lft[new_lid] = port_num;
	if (!(p_osm->subn.opt.port_profile_switch_nodes && port_guid &&
	      osm_get_switch_by_guid(&p_osm->subn, port_guid)))
		osm_switch_count_path(p_sw, port_num);

	OSM_LOG(&p_osm->log, OSM_LOG_DEBUG,
		"route 0x%04x(was 0x%04x) %u 0x%016" PRIx64
		" is added to switch 0x%016" PRIx64 "\n",
		new_lid, lid, port_num, cl_ntoh64(port_guid),
		cl_ntoh64(osm_node_get_node_guid(p_sw->p_node)));
}

static void add_lid_hops(osm_opensm_t * p_osm, osm_switch_t * p_sw,
			 uint16_t lid, ib_net64_t guid,
			 uint8_t hops[], unsigned len)
{
	uint16_t new_lid;
	uint8_t i;

	new_lid = guid ? remap_lid(p_osm, lid, guid) : lid;
	if (len > p_sw->num_ports)
		len = p_sw->num_ports;

	for (i = 0; i < len; i++)
		osm_switch_set_hops(p_sw, lid, i, hops[i]);
}

static int do_ucast_file_load(void *context)
{
	char line[1024];
	char *file_name;
	FILE *file;
	ib_net64_t sw_guid, port_guid;
	osm_opensm_t *p_osm = context;
	osm_switch_t *p_sw;
	uint16_t lid;
	uint8_t port_num;
	unsigned lineno;

	file_name = p_osm->subn.opt.lfts_file;
	if (!file_name) {
		OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
			"LFTs file name is not given; "
			"using default routing algorithm\n");
		return 1;
	}

	file = fopen(file_name, "r");
	if (!file) {
		OSM_LOG(&p_osm->log, OSM_LOG_ERROR | OSM_LOG_SYS, "ERR 6302: "
			"cannot open ucast dump file \'%s\': %m\n", file_name);
		return -1;
	}

	lineno = 0;
	p_sw = NULL;

	while (fgets(line, sizeof(line) - 1, file) != NULL) {
		char *p, *q;
		lineno++;

		p = line;
		while (isspace(*p))
			p++;

		if (*p == '#')
			continue;

		if (!strncmp(p, "Multicast mlids", 15)) {
			OSM_LOG(&p_osm->log, OSM_LOG_ERROR | OSM_LOG_SYS,
				"ERR 6303: "
				"Multicast dump file detected; "
				"skipping parsing. Using default "
				"routing algorithm\n");
		} else if (!strncmp(p, "Unicast lids", 12)) {
			if (p_sw)
				osm_ucast_mgr_set_fwd_table(&p_osm->sm.
							    ucast_mgr, p_sw);
			q = strstr(p, " guid 0x");
			if (!q) {
				OSM_LOG(&p_osm->log, OSM_LOG_ERROR,
					"PARSE ERROR: %s:%u: "
					"cannot parse switch definition\n",
					file_name, lineno);
				return -1;
			}
			p = q + 8;
			sw_guid = strtoull(p, &q, 16);
			if (q == p || !isspace(*q)) {
				OSM_LOG(&p_osm->log, OSM_LOG_ERROR,
					"PARSE ERROR: %s:%u: "
					"cannot parse switch guid: \'%s\'\n",
					file_name, lineno, p);
				return -1;
			}
			sw_guid = cl_hton64(sw_guid);

			p_sw = osm_get_switch_by_guid(&p_osm->subn, sw_guid);
			if (!p_sw) {
				OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
					"cannot find switch %016" PRIx64 "\n",
					cl_ntoh64(sw_guid));
				continue;
			}
			memset(p_sw->new_lft, OSM_NO_PATH,
			       IB_LID_UCAST_END_HO + 1);
		} else if (p_sw && !strncmp(p, "0x", 2)) {
			p += 2;
			lid = (uint16_t) strtoul(p, &q, 16);
			if (q == p || !isspace(*q)) {
				OSM_LOG(&p_osm->log, OSM_LOG_ERROR,
					"PARSE ERROR: %s:%u: "
					"cannot parse lid: \'%s\'\n",
					file_name, lineno, p);
				return -1;
			}
			p = q;
			while (isspace(*p))
				p++;
			port_num = (uint8_t) strtoul(p, &q, 10);
			if (q == p || !isspace(*q)) {
				OSM_LOG(&p_osm->log, OSM_LOG_ERROR,
					"PARSE ERROR: %s:%u: "
					"cannot parse port: \'%s\'\n",
					file_name, lineno, p);
				return -1;
			}
			p = q;
			/* additionally try to exract guid */
			q = strstr(p, " portguid 0x");
			if (!q) {
				OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
					"PARSE WARNING: %s:%u: "
					"cannot find port guid "
					"(maybe broken dump): \'%s\'\n",
					file_name, lineno, p);
				port_guid = 0;
			} else {
				p = q + 12;
				port_guid = strtoull(p, &q, 16);
				if (q == p || (!isspace(*q) && *q != ':')) {
					OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
						"PARSE WARNING: %s:%u: "
						"cannot parse port guid "
						"(maybe broken dump): \'%s\'\n",
						file_name, lineno, p);
					port_guid = 0;
				}
			}
			port_guid = cl_hton64(port_guid);
			add_path(p_osm, p_sw, lid, port_num, port_guid);
		}
	}

	if (p_sw)
		osm_ucast_mgr_set_fwd_table(&p_osm->sm.ucast_mgr, p_sw);

	fclose(file);
	return 0;
}

static int do_lid_matrix_file_load(void *context)
{
	char line[1024];
	uint8_t hops[256];
	char *file_name;
	FILE *file;
	ib_net64_t guid;
	osm_opensm_t *p_osm = context;
	osm_switch_t *p_sw;
	unsigned lineno;
	uint16_t lid;

	file_name = p_osm->subn.opt.lid_matrix_dump_file;
	if (!file_name) {
		OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
			"lid matrix file name is not given; "
			"using default lid matrix generation algorithm\n");
		return 1;
	}

	file = fopen(file_name, "r");
	if (!file) {
		OSM_LOG(&p_osm->log, OSM_LOG_ERROR | OSM_LOG_SYS, "ERR 6305: "
			"cannot open lid matrix file \'%s\': %m\n", file_name);
		return -1;
	}

	lineno = 0;
	p_sw = NULL;

	while (fgets(line, sizeof(line) - 1, file) != NULL) {
		char *p, *q;
		lineno++;

		p = line;
		while (isspace(*p))
			p++;

		if (*p == '#')
			continue;

		if (!strncmp(p, "Switch", 6)) {
			q = strstr(p, " guid 0x");
			if (!q) {
				OSM_LOG(&p_osm->log, OSM_LOG_ERROR,
					"PARSE ERROR: %s:%u: "
					"cannot parse switch definition\n",
					file_name, lineno);
				return -1;
			}
			p = q + 8;
			guid = strtoull(p, &q, 16);
			if (q == p || !isspace(*q)) {
				OSM_LOG(&p_osm->log, OSM_LOG_ERROR,
					"PARSE ERROR: %s:%u: "
					"cannot parse switch guid: \'%s\'\n",
					file_name, lineno, p);
				return -1;
			}
			guid = cl_hton64(guid);

			p_sw = osm_get_switch_by_guid(&p_osm->subn, guid);
			if (!p_sw) {
				OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
					"cannot find switch %016" PRIx64 "\n",
					cl_ntoh64(guid));
				continue;
			}
		} else if (p_sw && !strncmp(p, "0x", 2)) {
			unsigned long num;
			unsigned len = 0;

			memset(hops, 0xff, sizeof(hops));

			p += 2;
			num = strtoul(p, &q, 16);
			if (num > 0xffff || q == p ||
			    (*q != ':' && !isspace(*q))) {
				OSM_LOG(&p_osm->log, OSM_LOG_ERROR,
					"PARSE ERROR: %s:%u: "
					"cannot parse lid: \'%s\'\n",
					file_name, lineno, p);
				return -1;
			}
			/* Just checked the range, so casting is safe */
			lid = (uint16_t) num;
			p = q;
			while (isspace(*p) || *p == ':')
				p++;
			while (len < 256 && *p && *p != '#') {
				num = strtoul(p, &q, 16);
				if (num > 0xff || q == p) {
					OSM_LOG(&p_osm->log, OSM_LOG_ERROR,
						"PARSE ERROR: %s:%u: "
						"cannot parse hops number: \'%s\'\n",
						file_name, lineno, p);
					return -1;
				}
				/* Just checked the range, so casting is safe */
				hops[len++] = (uint8_t) num;
				p = q;
				while (isspace(*p))
					p++;
			}
			/* additionally try to extract guid */
			q = strstr(p, " portguid 0x");
			if (!q) {
				OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
					"PARSE WARNING: %s:%u: "
					"cannot find port guid "
					"(maybe broken dump): \'%s\'\n",
					file_name, lineno, p);
				guid = 0;
			} else {
				p = q + 12;
				guid = strtoull(p, &q, 16);
				if (q == p || !isspace(*q)) {
					OSM_LOG(&p_osm->log, OSM_LOG_VERBOSE,
						"PARSE WARNING: %s:%u: "
						"cannot parse port guid "
						"(maybe broken dump): \'%s\'\n",
						file_name, lineno, p);
					guid = 0;
				}
			}
			guid = cl_hton64(guid);
			add_lid_hops(p_osm, p_sw, lid, guid, hops, len);
		}
	}

	fclose(file);
	return 0;
}

int osm_ucast_file_setup(struct osm_routing_engine *r, osm_opensm_t *osm)
{
	r->context = osm;
	r->build_lid_matrices = do_lid_matrix_file_load;
	r->ucast_build_fwd_tables = do_ucast_file_load;
	return 0;
}