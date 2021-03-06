
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

#include <sys/poll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <dirent.h>
#include <stdlib.h>
#include <ctype.h>

#include "umad.h"

#define IB_OPENIB_OUI                 (0x001405)

#ifdef HAVE_VALGRIND_MEMCHECK_H

#  include <valgrind/memcheck.h>

#  ifndef VALGRIND_MAKE_MEM_DEFINED
#    warning "Valgrind support requested, but VALGRIND_MAKE_MEM_DEFINED not available"
#  endif

#endif /* HAVE_VALGRIND_MEMCHECK_H */

#ifndef VALGRIND_MAKE_MEM_DEFINED
#  define VALGRIND_MAKE_MEM_DEFINED(addr,len)
#endif

typedef struct ib_user_mad_reg_req {
	uint32_t id;
	uint32_t method_mask[4];
	uint8_t  qpn;
	uint8_t  mgmt_class;
	uint8_t  mgmt_class_version;
	uint8_t  oui[3];
	uint8_t  rmpp_version;
} ib_user_mad_reg_req_t;

#define TRACE	if (umaddebug)	IBWARN
#define DEBUG	if (umaddebug)	IBWARN

int umaddebug = 0;

#define UMAD_DEV_FILE_SZ	256

static char *def_ca_name = "mthca0";
static int def_ca_port = 1;

static unsigned abi_version;
static unsigned new_user_mad_api;

/*************************************
 * Port
 */
static int
find_cached_ca(char *ca_name, umad_ca_t *ca)
{
	return 0;	/* caching not implemented yet */
}

static int
put_ca(umad_ca_t *ca)
{
	return 0;	/* caching not implemented yet */
}

static int
release_port(umad_port_t *port)
{
	free(port->pkeys);
	port->pkeys = NULL;
	port->pkeys_size = 0;
	return 0;
}

static int check_for_digit_name(const struct dirent *dent)
{
	const char *p = dent->d_name;
	while (*p && isdigit(*p))
		p++;
	return *p ? 0 : 1;
}

static int
get_port(char *ca_name, char *dir, int portnum, umad_port_t *port)
{
	char port_dir[256];
	uint8_t gid[16];
	struct dirent **namelist = NULL;
	int i, len, ret = 0;

	strncpy(port->ca_name, ca_name, sizeof port->ca_name - 1);
	port->portnum = portnum;
	port->pkeys = NULL;

	len = snprintf(port_dir, sizeof(port_dir), "%s/%d", dir, portnum);
	if (len < 0 || len > sizeof(port_dir))
		goto clean;

	if (sys_read_uint(port_dir, SYS_PORT_LMC, &port->lmc) < 0)
		goto clean;
	if (sys_read_uint(port_dir, SYS_PORT_SMLID, &port->sm_lid) < 0)
		goto clean;
	if (sys_read_uint(port_dir, SYS_PORT_SMSL, &port->sm_sl) < 0)
		goto clean;
	if (sys_read_uint(port_dir, SYS_PORT_LID, &port->base_lid) < 0)
		goto clean;
	if (sys_read_uint(port_dir, SYS_PORT_STATE, &port->state) < 0)
		goto clean;
	if (sys_read_uint(port_dir, SYS_PORT_PHY_STATE, &port->phys_state) < 0)
		goto clean;
	if (sys_read_uint(port_dir, SYS_PORT_RATE, &port->rate) < 0)
		goto clean;
	if (sys_read_uint64(port_dir, SYS_PORT_CAPMASK, &port->capmask) < 0)
		goto clean;

	port->capmask = htonl(port->capmask);

	if (sys_read_gid(port_dir, SYS_PORT_GID, gid) < 0)
		goto clean;

	memcpy(&port->gid_prefix, gid, sizeof port->gid_prefix);
	memcpy(&port->port_guid, gid + 8, sizeof port->port_guid);

	snprintf(port_dir + len, sizeof(port_dir) - len, "/pkeys");
	ret = sys_scandir(port_dir, &namelist, check_for_digit_name, NULL);
	if (ret <= 0) {
		IBWARN("no pkeys found for %s:%u (at dir %s)...",
		       port->ca_name, port->portnum, port_dir);
		goto clean;
	}
	port->pkeys = calloc(ret, sizeof(port->pkeys[0]));
	if (!port->pkeys) {
		IBWARN("get_port: calloc failed: %s", strerror(errno));
		goto clean;
	}
	for (i = 0; i < ret ; i++) {
		unsigned idx, val;
		idx = strtoul(namelist[i]->d_name, NULL, 0);
		sys_read_uint(port_dir, namelist[i]->d_name, &val);
		port->pkeys[idx] = val;
		free(namelist[i]);
	}
	port->pkeys_size = ret;
	free(namelist);
	namelist = NULL;
	port_dir[len] = '\0';

	/* FIXME: handle gids */

	return 0;

clean:
	if (namelist) {
		for (i = 0; i < ret ; i++)
			free(namelist[i]);
		free(namelist);
	}
	if (port->pkeys)
		free(port->pkeys);
	return -EIO;
}

static int
release_ca(umad_ca_t *ca)
{
	int i;

	for (i = 0; i <= ca->numports; i++) {
		if (!ca->ports[i])
			continue;
		release_port(ca->ports[i]);
		free(ca->ports[i]);
		ca->ports[i] = 0;
	}
	return 0;
}

/*
 * if *port > 0, check ca[port] state. Otherwise set *port to
 * the first port that is active, and if such is not found, to
 * the first port that is link up and if none are linkup, then
 * the first port that is not disabled.  Otherwise return -1.
 */
static int
resolve_ca_port(char *ca_name, int *port)
{
	umad_ca_t ca;
	int active = -1, up = -1;
	int i;

	TRACE("checking ca '%s'", ca_name);

	if (umad_get_ca(ca_name, &ca) < 0)
		return -1;

	if (ca.node_type == 2) {
		*port = 0;	/* switch sma port 0 */
		return 1;
	}

	if (*port > 0) {	/* check only the port the user wants */
		if (*port > ca.numports)
			return -1;
		if (!ca.ports[*port])
			return -1;
		if (ca.ports[*port]->state == 4)
			return 1;
		if (ca.ports[*port]->phys_state != 3)
			return 0;
		return -1;
	}

	for (i = 0; i <= ca.numports; i++) {
		DEBUG("checking port %d", i);
		if (!ca.ports[i])
			continue;
		if (up < 0 && ca.ports[i]->phys_state == 5)
			up = *port = i;
		if (ca.ports[i]->state == 4) {
			active = *port = i;
			DEBUG("found active port %d", i);
			break;
		}
	}

	if (active == -1 && up == -1) { /* no active or linkup port found */
		for (i = 0; i <= ca.numports; i++) {
			DEBUG("checking port %d", i);
			if (!ca.ports[i])
				continue;
			if (ca.ports[i]->phys_state != 3) {
				up = *port = i;
				break;
			}
		}
	}

	release_ca(&ca);

	if (active >= 0)
		return 1;
	if (up >= 0)
		return 0;
	return -1;
}

static char *
resolve_ca_name(char *ca_name, int *best_port)
{
	static char names[UMAD_MAX_DEVICES][UMAD_CA_NAME_LEN];
	int phys_found = -1, port_found = 0, port, port_type;
	int caidx, n;

	if (ca_name && (!best_port || *best_port))
		return ca_name;

	if (ca_name) {
		if (resolve_ca_port(ca_name, best_port) < 0)
			return 0;
		return ca_name;
	}

	/* Get the list of CA names */
	if ((n = umad_get_cas_names((void *)names, 20)) < 0)
		return 0;

	/* Find the first existing CA with an active port */
	for (caidx = 0; caidx < n; caidx++) {
		TRACE("checking ca '%s'", names[caidx]);

		port = best_port ? *best_port : 0;
		if ((port_type = resolve_ca_port(names[caidx], &port)) < 0)
			continue;

		DEBUG("found ca %s with port %d type %d",
			names[caidx], port, port_type);

		if (port_type > 0) {
			if (best_port)
				*best_port = port;
			DEBUG("found ca %s with active port %d",
			      names[caidx], port);
			return (char *)(names + caidx);
		}

		if (phys_found == -1) {
			phys_found = caidx;
			port_found = port;
		}
	}

	DEBUG("phys found %d on %s port %d",
		phys_found, phys_found >=0 ? names[phys_found] : 0, port_found);
	if (phys_found >= 0) {
		if (best_port)
			*best_port = port_found;
		return names[phys_found];
	}

	if (best_port)
		*best_port = def_ca_port;
	return def_ca_name;
}

static int
get_ca(char *ca_name, umad_ca_t *ca)
{
#ifdef __linux__
	DIR *dir;
#endif
	char dir_name[256];
	struct dirent **namelist;
	int r, i, ret;
	int portnum;

	strncpy(ca->ca_name, ca_name, sizeof ca->ca_name);

	snprintf(dir_name, sizeof(dir_name), "%s/%s", SYS_INFINIBAND,
		 ca->ca_name);

	if ((r = sys_read_uint(dir_name, SYS_NODE_TYPE, &ca->node_type)) < 0)
		return r;
	if (sys_read_string(dir_name, SYS_CA_FW_VERS, ca->fw_ver,
			    sizeof ca->fw_ver) < 0)
		ca->fw_ver[0] = '\0';
	if (sys_read_string(dir_name, SYS_CA_HW_VERS, ca->hw_ver,
			    sizeof ca->hw_ver) < 0)
		ca->hw_ver[0] = '\0';
	if ((r = sys_read_string(dir_name, SYS_CA_TYPE, ca->ca_type,
				 sizeof ca->ca_type)) < 0)
		ca->ca_type[0] = '\0';
	if ((r = sys_read_guid(dir_name, SYS_CA_NODE_GUID, &ca->node_guid)) < 0)
		return r;
	if ((r = sys_read_guid(dir_name, SYS_CA_SYS_GUID, &ca->system_guid)) < 0)
		return r;

	snprintf(dir_name, sizeof(dir_name), "%s/%s/%s",
		SYS_INFINIBAND, ca->ca_name, SYS_CA_PORTS_DIR);

#ifdef __linux__
	if (!(dir = opendir(dir_name)))
		return -ENOENT;
#endif

	if ((r = sys_scandir(dir_name, &namelist, 0, alphasort)) < 0) {
		ret = errno < 0 ? errno : -EIO;
		goto error;
	}

	ret = 0;
	ca->numports = 0;
	memset(ca->ports, 0, sizeof ca->ports);
	for (i = 0; i < r; i++) {
		portnum = 0;
		if (!strcmp(".", namelist[i]->d_name) ||
		    !strcmp("..", namelist[i]->d_name))
			continue;
		if (strcmp("0", namelist[i]->d_name) &&
		    ((portnum = atoi(namelist[i]->d_name)) <= 0 ||
		     portnum >= UMAD_CA_MAX_PORTS)) {
			ret = -EIO;
			goto clean;
		}
		if (!(ca->ports[portnum] = calloc(1, sizeof(*ca->ports[portnum])))) {
			ret = -ENOMEM;
			goto clean;
		}
		if (get_port(ca_name, dir_name, portnum, ca->ports[portnum]) < 0) {
			free(ca->ports[portnum]);
			ca->ports[portnum] = NULL;
			ret = -EIO;
			goto clean;
		}
		if (ca->numports < portnum)
			ca->numports = portnum;
	}

	for (i = 0; i < r; i++)
		free(namelist[i]);
	free(namelist);

#ifdef __linux__
	closedir(dir);
#endif
	put_ca(ca);
	return 0;

clean:
	for (i = 0; i < r; i++)
		free(namelist[i]);
	free(namelist);
error:
#ifdef __linux__
	closedir(dir);
#endif
	release_ca(ca);

	return ret;
}

static int
umad_id_to_dev(int umad_id, char *dev, unsigned *port)
{
	char path[256];
	int r;

	snprintf(path, sizeof(path), SYS_INFINIBAND_MAD "/umad%d", umad_id);

	if ((r = sys_read_string(path, SYS_IB_MAD_DEV, dev, UMAD_CA_NAME_LEN)) < 0)
		return r;

	if ((r = sys_read_uint(path, SYS_IB_MAD_PORT, port)) < 0)
		return r;

	return 0;
}

static int
dev_to_umad_id(char *dev, unsigned port)
{
	char umad_dev[UMAD_CA_NAME_LEN];
	unsigned umad_port;
	int id;

	for (id = 0; id < UMAD_MAX_PORTS; id++) {
		if (umad_id_to_dev(id, umad_dev, &umad_port) < 0)
			continue;
		if (strncmp(dev, umad_dev, UMAD_CA_NAME_LEN))
			continue;
		if (port != umad_port)
			continue;

		DEBUG("mapped %s %d to %d", dev, port, id);
		return id;
	}

	return -1;	/* not found */
}

/*******************************
 * Public interface
 */

int
umad_init(void)
{
	TRACE("umad_init");
	if (sys_read_uint(IB_UMAD_ABI_DIR, IB_UMAD_ABI_FILE, &abi_version) < 0) {
		IBWARN("can't read ABI version from %s/%s (%m): is ib_umad module loaded?",
			IB_UMAD_ABI_DIR, IB_UMAD_ABI_FILE);
		return -1;
	}
	if (abi_version < IB_UMAD_ABI_VERSION) {
		IBWARN("wrong ABI version: %s/%s is %d but library minimal ABI is %d",
			IB_UMAD_ABI_DIR, IB_UMAD_ABI_FILE, abi_version, IB_UMAD_ABI_VERSION);
		return -1;
	}
	return 0;
}

int
umad_done(void)
{
	TRACE("umad_done");
	/* FIXME - verify that all ports are closed */
	return 0;
}

static unsigned is_ib_type(char *ca_name)
{
	char dir_name[256];
	unsigned type;

	snprintf(dir_name, sizeof(dir_name), "%s/%s", SYS_INFINIBAND, ca_name);

	if (sys_read_uint(dir_name, SYS_NODE_TYPE, &type) < 0)
		return 0;

	return type >= 1 && type <= 3 ? 1 : 0;
}

int
umad_get_cas_names(char cas[][UMAD_CA_NAME_LEN], int max)
{
	struct dirent **namelist;
	int n, i, j = 0;

	TRACE("max %d", max);

	n = sys_scandir(SYS_INFINIBAND, &namelist, NULL, alphasort);
	if (n > 0) {
		for (i = 0; i < n; i++) {
			if (strcmp(namelist[i]->d_name, ".") &&
			    strcmp(namelist[i]->d_name, "..")) {
				if (j < max && is_ib_type(namelist[i]->d_name))
					strncpy(cas[j++], namelist[i]->d_name,
						UMAD_CA_NAME_LEN);
			}
			free(namelist[i]);
		}
		DEBUG("return %d cas", j);
	} else {
		/* Is this still needed ? */
		strncpy((char *)cas, def_ca_name, UMAD_CA_NAME_LEN);
		DEBUG("return 1 ca");
		j = 1;
	}
	if (n >= 0)
		free(namelist);
	return j;
}

int
umad_get_ca_portguids(char *ca_name, uint64_t *portguids, int max)
{
	umad_ca_t ca;
	int ports = 0, i;

	TRACE("ca name %s max port guids %d", ca_name, max);
	if (!(ca_name = resolve_ca_name(ca_name, 0)))
		return -ENODEV;

	if (umad_get_ca(ca_name, &ca) < 0)
		return -1;

	if (portguids) {
		if (ca.numports + 1 > max) {
			release_ca(&ca);
			return -ENOMEM;
		}

		for (i = 0; i <= ca.numports; i++)
			portguids[ports++] = ca.ports[i] ? ca.ports[i]->port_guid : 0;
	}

	release_ca(&ca);
	DEBUG("%s: %d ports", ca_name, ports);

	return ports;
}

int
umad_get_issm_path(char *ca_name, int portnum, char path[], int max)
{
	int umad_id;

	TRACE("ca %s port %d", ca_name, portnum);

	if (!(ca_name = resolve_ca_name(ca_name, &portnum)))
		return -ENODEV;

	if ((umad_id = dev_to_umad_id(ca_name, portnum)) < 0)
		return -EINVAL;

	snprintf(path, max, "%s/issm%u", UMAD_DEV_DIR , umad_id);

	return 0;
}

int
umad_open_port(char *ca_name, int portnum)
{
	char dev_file[UMAD_DEV_FILE_SZ];
	int umad_id, fd;

	TRACE("ca %s port %d", ca_name, portnum);

	if (!(ca_name = resolve_ca_name(ca_name, &portnum)))
		return -ENODEV;

	DEBUG("opening %s port %d", ca_name, portnum);

	if ((umad_id = dev_to_umad_id(ca_name, portnum)) < 0)
		return -EINVAL;

	snprintf(dev_file, sizeof(dev_file), "%s/umad%d",
		 UMAD_DEV_DIR , umad_id);

	if ((fd = open(dev_file, O_RDWR|O_NONBLOCK)) < 0) {
		DEBUG("open %s failed: %s", dev_file, strerror(errno));
		return -EIO;
	}

	if (abi_version > 5 || !ioctl(fd, IB_USER_MAD_ENABLE_PKEY, NULL))
		new_user_mad_api = 1;
	else
		new_user_mad_api = 0;

	DEBUG("opened %s fd %d portid %d", dev_file, fd, umad_id);
	return fd;
}

int
umad_get_ca(char *ca_name, umad_ca_t *ca)
{
	int r;

	TRACE("ca_name %s", ca_name);
	if (!(ca_name = resolve_ca_name(ca_name, 0)))
		return -ENODEV;

	if (find_cached_ca(ca_name, ca) > 0)
		return 0;

	if ((r = get_ca(ca_name, ca)) < 0)
		return r;

	DEBUG("opened %s", ca_name);
	return 0;
}

int
umad_release_ca(umad_ca_t *ca)
{
	int r;

	TRACE("ca_name %s", ca->ca_name);
	if (!ca)
		return -ENODEV;

	if ((r = release_ca(ca)) < 0)
		return r;

	DEBUG("releasing %s", ca->ca_name);
	return 0;
}

int
umad_get_port(char *ca_name, int portnum, umad_port_t *port)
{
	char dir_name[256];

	TRACE("ca_name %s portnum %d", ca_name, portnum);

	if (!(ca_name = resolve_ca_name(ca_name, &portnum)))
		return -ENODEV;

	snprintf(dir_name, sizeof(dir_name), "%s/%s/%s",
		SYS_INFINIBAND, ca_name, SYS_CA_PORTS_DIR);

	return get_port(ca_name, dir_name, portnum, port);
}

int
umad_release_port(umad_port_t *port)
{
	int r;

	TRACE("port %s:%d", port->ca_name, port->portnum);
	if (!port)
		return -ENODEV;

	if ((r = release_port(port)) < 0)
		return r;

	DEBUG("releasing %s:%d", port->ca_name, port->portnum);
	return 0;
}

int
umad_close_port(int fd)
{
	close(fd);
	DEBUG("closed fd %d", fd);
	return 0;
}

void *
umad_get_mad(void *umad)
{
	return new_user_mad_api ? ((struct ib_user_mad *)umad)->data :
		(void *)&((struct ib_user_mad *)umad)->addr.pkey_index;
}

size_t
umad_size(void)
{
	return new_user_mad_api ? sizeof (struct ib_user_mad) :
		sizeof(struct ib_user_mad) - 8;
}

int
umad_set_grh(void *umad, void *mad_addr)
{
	struct ib_user_mad *mad = umad;
	struct ib_mad_addr *addr = mad_addr;

	if (mad_addr) {
		mad->addr.grh_present = 1;
		memcpy(mad->addr.gid, addr->gid, 16);
		mad->addr.flow_label = htonl(addr->flow_label);
		mad->addr.hop_limit = addr->hop_limit;
		mad->addr.traffic_class = addr->traffic_class;
	} else
		mad->addr.grh_present = 0;
	return 0;
}

int
umad_set_pkey(void *umad, int pkey_index)
{
	struct ib_user_mad *mad = umad;

	if (new_user_mad_api)
		mad->addr.pkey_index = pkey_index;

	return 0;
}

int
umad_get_pkey(void *umad)
{
	struct ib_user_mad *mad = umad;

	if (new_user_mad_api)
		return mad->addr.pkey_index;

	return 0;
}

int
umad_set_addr(void *umad, int dlid, int dqp, int sl, int qkey)
{
	struct ib_user_mad *mad = umad;

	TRACE("umad %p dlid %d dqp %d sl %d, qkey %x",
	      umad, dlid, dqp, sl, qkey);
	mad->addr.qpn = htonl(dqp);
	mad->addr.lid = htons(dlid);
	mad->addr.qkey = htonl(qkey);
	mad->addr.sl = sl;

	return 0;
}

int
umad_set_addr_net(void *umad, int dlid, int dqp, int sl, int qkey)
{
	struct ib_user_mad *mad = umad;

	TRACE("umad %p dlid %d dqp %d sl %d qkey %x",
	      umad, ntohs(dlid), ntohl(dqp), sl, ntohl(qkey));
	mad->addr.qpn = dqp;
	mad->addr.lid = dlid;
	mad->addr.qkey = qkey;
	mad->addr.sl = sl;

	return 0;
}

int
umad_send(int fd, int agentid, void *umad, int length,
	  int timeout_ms, int retries)
{
	struct ib_user_mad *mad = umad;
	int n;

	TRACE("fd %d agentid %d umad %p timeout %u",
	      fd, agentid, umad, timeout_ms);
	errno = 0;

	mad->timeout_ms = timeout_ms;
	mad->retries = retries;
	mad->agent_id = agentid;

	if (umaddebug > 1)
		umad_dump(mad);

	n = write(fd, mad, length + umad_size());
	if (n == length + umad_size())
		return 0;

	DEBUG("write returned %d != sizeof umad %zu + length %d (%m)",
	      n, umad_size(), length);
	if (!errno)
		errno = EIO;
	return -EIO;
}

static int
dev_poll(int fd, int timeout_ms)
{
	struct pollfd ufds;
	int n;

	ufds.fd     = fd;
	ufds.events = POLLIN;

	if ((n = poll(&ufds, 1, timeout_ms)) == 1)
		return 0;

	if (n == 0)
		return -ETIMEDOUT;

	return -EIO;
}

int
umad_recv(int fd, void *umad, int *length, int timeout_ms)
{
	struct ib_user_mad *mad = umad;
	int n;

	errno = 0;
	TRACE("fd %d umad %p timeout %u", fd, umad, timeout_ms);

	if (!umad || !length) {
		errno = EINVAL;
		return -EINVAL;
	}

	if (timeout_ms && (n = dev_poll(fd, timeout_ms)) < 0) {
		if (!errno)
			errno = -n;
		return n;
	}

	n = read(fd, umad, umad_size() + *length);

	VALGRIND_MAKE_MEM_DEFINED(umad, umad_size() + *length);

	if ((n >= 0) && (n <= umad_size() + *length)) {
		DEBUG("mad received by agent %d length %d", mad->agent_id, n);
		if (n > umad_size())
			*length = n - umad_size();
		else
			*length = 0;
		return mad->agent_id;
	}

	if (n == -EWOULDBLOCK) {
		if (!errno)
			errno = EWOULDBLOCK;
		return n;
	}

	DEBUG("read returned %zu > sizeof umad %zu + length %d (%m)",
	      mad->length - umad_size(), umad_size(), *length);

	*length = mad->length - umad_size();
	if (!errno)
		errno = EIO;
	return -errno;
}

int
umad_poll(int fd, int timeout_ms)
{
	TRACE("fd %d timeout %u", fd, timeout_ms);
	return dev_poll(fd, timeout_ms);
}

int
umad_get_fd(int fd)
{
	TRACE("fd %d", fd);
	return fd;
}

int
umad_register_oui(int fd, int mgmt_class, uint8_t rmpp_version,
		  uint8_t oui[3], long method_mask[])
{
	struct ib_user_mad_reg_req req;

	TRACE("fd %d mgmt_class %u rmpp_version %d oui 0x%x%x%x method_mask %p",
		fd, mgmt_class, (int)rmpp_version, (int)oui[0], (int)oui[1],
		(int)oui[2], method_mask);

	if (mgmt_class < 0x30 || mgmt_class > 0x4f) {
		DEBUG("mgmt class %d not in vendor range 2", mgmt_class);
		return -EINVAL;
	}

	req.qpn = 1;
	req.mgmt_class = mgmt_class;
	req.mgmt_class_version = 1;
	memcpy(req.oui, oui, sizeof req.oui);
	req.rmpp_version = rmpp_version;

	if (method_mask)
		memcpy(req.method_mask, method_mask, sizeof req.method_mask);
	else
		memset(req.method_mask, 0, sizeof req.method_mask);

	VALGRIND_MAKE_MEM_DEFINED(&req, sizeof req);

	if (!ioctl(fd, IB_USER_MAD_REGISTER_AGENT, (void *)&req)) {
		DEBUG("fd %d registered to use agent %d qp %d class 0x%x oui %p",
			fd, req.id, req.qpn, req.mgmt_class, oui);
		return req.id; 		/* return agentid */
	}

	DEBUG("fd %d registering qp %d class 0x%x version %d oui %p failed: %m",
		fd, req.qpn, req.mgmt_class, req.mgmt_class_version, oui);
	return -EPERM;
}

int
umad_register(int fd, int mgmt_class, int mgmt_version,
	      uint8_t rmpp_version, long method_mask[])
{
	struct ib_user_mad_reg_req req;
	uint32_t oui = htonl(IB_OPENIB_OUI);
	int qp;

	TRACE("fd %d mgmt_class %u mgmt_version %u rmpp_version %d method_mask %p",
		fd, mgmt_class, mgmt_version, rmpp_version, method_mask);

	req.qpn = qp = (mgmt_class == 0x1 || mgmt_class == 0x81) ? 0 : 1;
	req.mgmt_class = mgmt_class;
	req.mgmt_class_version = mgmt_version;
	req.rmpp_version = rmpp_version;

	if (method_mask)
		memcpy(req.method_mask, method_mask, sizeof req.method_mask);
	else
		memset(req.method_mask, 0, sizeof req.method_mask);

	memcpy(&req.oui, (char *)&oui + 1, sizeof req.oui);

	VALGRIND_MAKE_MEM_DEFINED(&req, sizeof req);

	if (!ioctl(fd, IB_USER_MAD_REGISTER_AGENT, (void *)&req)) {
		DEBUG("fd %d registered to use agent %d qp %d",
		      fd, req.id, qp);
		return req.id; 		/* return agentid */
	}

	DEBUG("fd %d registering qp %d class 0x%x version %d failed: %m",
		fd, qp, mgmt_class, mgmt_version);
	return -EPERM;
}

int
umad_unregister(int fd, int agentid)
{
	TRACE("fd %d unregistering agent %d", fd, agentid);
	return ioctl(fd, IB_USER_MAD_UNREGISTER_AGENT, &agentid);
}

int
umad_status(void *umad)
{
	struct ib_user_mad *mad = umad;

	return mad->status;
}

ib_mad_addr_t *
umad_get_mad_addr(void *umad)
{
	struct ib_user_mad *mad = umad;

	return &mad->addr;
}

int
umad_debug(int level)
{
	if (level >= 0)
		umaddebug = level;
	return umaddebug;
}

void
umad_addr_dump(ib_mad_addr_t *addr)
{
#define HEX(x)  ((x) < 10 ? '0' + (x) : 'a' + ((x) -10))
	char gid_str[64];
	int i;

	for (i = 0; i < sizeof addr->gid; i++) {
		gid_str[i*2] = HEX(addr->gid[i] >> 4);
		gid_str[i*2+1] = HEX(addr->gid[i] & 0xf);
	}
	gid_str[i*2] = 0;
	IBWARN("qpn %d qkey 0x%x lid 0x%x sl %d\n"
		"grh_present %d gid_index %d hop_limit %d traffic_class %d flow_label 0x%x pkey_index 0x%x\n"
		"Gid 0x%s",
		ntohl(addr->qpn), ntohl(addr->qkey), ntohs(addr->lid), addr->sl,
		addr->grh_present, (int)addr->gid_index, (int)addr->hop_limit,
		(int)addr->traffic_class, addr->flow_label, addr->pkey_index,
		gid_str);
}

void
umad_dump(void *umad)
{
	struct ib_user_mad * mad = umad;

	IBWARN("agent id %d status %x timeout %d",
	     mad->agent_id, mad->status, mad->timeout_ms);
	umad_addr_dump(&mad->addr);
}