
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
 * Host Resources MIB implementation for bsnmpd.
 */

#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "hostres_snmp.h"
#include "hostres_oid.h"
#include "hostres_tree.h"

/* Internal id got after we'll register this module with the agent */
static u_int host_registration_id = 0;

/* This our hostres module */
static struct lmodule *hostres_module;

/* See the generated file hostres_oid.h */
static const struct asn_oid oid_host = OIDX_host;

/* descriptor to access kernel memory */
kvm_t *hr_kd;

/*
 * HOST RESOURCES mib module finalization hook.
 * Returns 0 on success, < 0 on error
 */
static int
hostres_fini(void)
{

	if (hr_kd != NULL)
		(void)kvm_close(hr_kd);

	fini_storage_tbl();
	fini_fs_tbl();
	fini_processor_tbl();
	fini_disk_storage_tbl();
	fini_device_tbl();
	fini_partition_tbl();
	fini_network_tbl();
	fini_printer_tbl();

	fini_swrun_tbl();
	fini_swins_tbl();

	fini_scalars();

	if (host_registration_id > 0)
		or_unregister(host_registration_id);

	HRDBG("done.");
	return (0);
}

/*
 * HOST RESOURCES mib module initialization hook.
 * Returns 0 on success, < 0 on error
 */
static int
hostres_init(struct lmodule *mod, int argc __unused, char *argv[] __unused)
{

	hostres_module = mod;

	/*
	 * NOTE: order of these calls is important here!
	 */
	if ((hr_kd = kvm_open(NULL, _PATH_DEVNULL, NULL, O_RDONLY,
	    "kvm_open")) == NULL) {
		syslog(LOG_ERR, "kvm_open failed: %m ");
		return (-1);
	}

	/*
	 * The order is relevant here, because some table depend on each other.
	 */
	init_device_tbl();

	/* populates partition table too */
	if (init_disk_storage_tbl()) {
		hostres_fini();
		return (-1);
	}
	init_processor_tbl();
	init_printer_tbl();

	/*
	 * populate storage and FS tables. Must be done after device
	 * initialisation because the FS refresh code calls into the
	 * partition refresh code.
	 */
	init_storage_tbl();


	/* also the hrSWRunPerfTable's support is initialized here */
	init_swrun_tbl();
	init_swins_tbl();

	HRDBG("done.");

	return (0);
}

/*
 * HOST RESOURCES mib module start operation
 * returns nothing
 */
static void
hostres_start(void)
{

	host_registration_id = or_register(&oid_host,
	    "The MIB module for Host Resource MIB (RFC 2790).",
	    hostres_module);

	start_device_tbl(hostres_module);
	start_processor_tbl(hostres_module);
	start_network_tbl();

        HRDBG("done.");
}

/* this identifies the HOST RESOURCES mib module */
const struct snmp_module config = {
	"This module implements the host resource mib (rfc 2790)",
	hostres_init,
	hostres_fini,
	NULL,			/* idle function, do not use it */
	NULL,
	NULL,
	hostres_start,
	NULL,                   /* proxy a PDU */
	hostres_ctree,          /* see the generated hostres_tree.h */
	hostres_CTREE_SIZE,     /* see the generated hostres_tree.h */
	NULL
};

/**
 * Make an SNMP DateAndTime from a struct tm. This should be in the library.
 */
int
make_date_time(u_char *str, const struct tm *tm, u_int decisecs)
{

	str[0] = (u_char)((tm->tm_year + 1900) >> 8);
	str[1] = (u_char)(tm->tm_year + 1900);
	str[2] = tm->tm_mon + 1;
	str[3] = tm->tm_mday;
	str[4] = tm->tm_hour;
	str[5] = tm->tm_min;
	str[6] = tm->tm_sec;
	str[7] = decisecs;
	if (tm->tm_gmtoff < 0)
		str[8] = '-';
	else
		str[8] = '+';

	str[9] = (u_char)(abs(tm->tm_gmtoff) / 3600);
	str[10] = (u_char)((abs(tm->tm_gmtoff) % 3600) / 60);

	return (11);
}