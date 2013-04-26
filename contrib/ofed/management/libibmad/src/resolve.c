
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#include <infiniband/common.h>
#include <infiniband/umad.h>
#include <mad.h>

#undef DEBUG
#define DEBUG 	if (ibdebug)	IBWARN

int
ib_resolve_smlid_via(ib_portid_t *sm_id, int timeout, const void *srcport)
{
	ib_portid_t self = {0};
	uint8_t portinfo[64];
	int lid;

	memset(sm_id, 0, sizeof(*sm_id));

	if (!smp_query_via(portinfo, &self, IB_ATTR_PORT_INFO,
			   0, 0, srcport))
		return -1;

	mad_decode_field(portinfo, IB_PORT_SMLID_F, &lid);

	return ib_portid_set(sm_id, lid, 0, 0);
}

int
ib_resolve_smlid(ib_portid_t *sm_id, int timeout)
{
	return ib_resolve_smlid_via(sm_id, timeout, NULL);
}

int
ib_resolve_guid_via(ib_portid_t *portid, uint64_t *guid, ib_portid_t *sm_id, int timeout, const void *srcport)
{
	ib_portid_t sm_portid;
	char buf[IB_SA_DATA_SIZE] = {0};

	if (!sm_id) {
		sm_id = &sm_portid;
		if (ib_resolve_smlid_via(sm_id, timeout, srcport) < 0)
			return -1;
	}
	if (*(uint64_t*)&portid->gid == 0)
		mad_set_field64(portid->gid, 0, IB_GID_PREFIX_F, IB_DEFAULT_SUBN_PREFIX);
	if (guid)
		mad_set_field64(portid->gid, 0, IB_GID_GUID_F, *guid);

	if ((portid->lid = ib_path_query_via(srcport, portid->gid, portid->gid, sm_id, buf)) < 0)
		return -1;

	return 0;
}

int
ib_resolve_portid_str_via(ib_portid_t *portid, char *addr_str, int dest_type, ib_portid_t *sm_id, const void *srcport)
{
	uint64_t guid;
	int lid;
	char *routepath;
	ib_portid_t selfportid = {0};
	int selfport = 0;

	switch (dest_type) {
	case IB_DEST_LID:
		lid = strtol(addr_str, 0, 0);
		if (!IB_LID_VALID(lid))
			return -1;
		return ib_portid_set(portid, lid, 0, 0);

	case IB_DEST_DRPATH:
		if (str2drpath(&portid->drpath, addr_str, 0, 0) < 0)
			return -1;
		return 0;

	case IB_DEST_GUID:
		if (!(guid = strtoull(addr_str, 0, 0)))
			return -1;

		/* keep guid in portid? */
		return ib_resolve_guid_via(portid, &guid, sm_id, 0, srcport);

	case IB_DEST_DRSLID:
		lid = strtol(addr_str, &routepath, 0);
		routepath++;
		if (!IB_LID_VALID(lid))
			return -1;
		ib_portid_set(portid, lid, 0, 0);

		/* handle DR parsing and set DrSLID to local lid */
		if (ib_resolve_self_via(&selfportid, &selfport, 0, srcport) < 0)
			return -1;
		if (str2drpath(&portid->drpath, routepath, selfportid.lid, 0) < 0)
			return -1;
		return 0;

	default:
		IBWARN("bad dest_type %d", dest_type);
	}

	return -1;
}

int
ib_resolve_portid_str(ib_portid_t *portid, char *addr_str, int dest_type, ib_portid_t *sm_id)
{
	return ib_resolve_portid_str_via(portid, addr_str, dest_type,
					 sm_id, NULL);
}

int
ib_resolve_self_via(ib_portid_t *portid, int *portnum, ibmad_gid_t *gid,
		    const void *srcport)
{
	ib_portid_t self = {0};
	uint8_t portinfo[64];
	uint8_t nodeinfo[64];
	uint64_t guid, prefix;

	if (!smp_query_via(nodeinfo, &self, IB_ATTR_NODE_INFO, 0, 0, srcport))
		return -1;

	if (!smp_query_via(portinfo, &self, IB_ATTR_PORT_INFO, 0, 0, srcport))
		return -1;

	mad_decode_field(portinfo, IB_PORT_LID_F, &portid->lid);
	mad_decode_field(portinfo, IB_PORT_GID_PREFIX_F, &prefix);
	mad_decode_field(nodeinfo, IB_NODE_PORT_GUID_F, &guid);

	if (portnum)
		mad_decode_field(nodeinfo, IB_NODE_LOCAL_PORT_F, portnum);
	if (gid) {
		mad_encode_field(*gid, IB_GID_PREFIX_F, &prefix);
		mad_encode_field(*gid, IB_GID_GUID_F, &guid);
	}
	return 0;
}

int
ib_resolve_self(ib_portid_t *portid, int *portnum, ibmad_gid_t *gid)
{
	return ib_resolve_self_via (portid, portnum, gid, NULL);
}