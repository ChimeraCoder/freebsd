
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

/* $Id$ */

/*! \file */

/* Principal Authors: DCL */

#include <config.h>

#include <isc/util.h>

#include <dns/log.h>

/*%
 * When adding a new category, be sure to add the appropriate
 * \#define to <dns/log.h>.
 */
LIBDNS_EXTERNAL_DATA isc_logcategory_t dns_categories[] = {
	{ "notify", 	0 },
	{ "database", 	0 },
	{ "security", 	0 },
	{ "_placeholder", 0 },
	{ "dnssec",	0 },
	{ "resolver",	0 },
	{ "xfer-in",	0 },
	{ "xfer-out",	0 },
	{ "dispatch",	0 },
	{ "lame-servers", 0 },
	{ "delegation-only", 0 },
	{ "edns-disabled", 0 },
	{ "rpz",	0 },
	{ NULL, 	0 }
};

/*%
 * When adding a new module, be sure to add the appropriate
 * \#define to <dns/log.h>.
 */
LIBDNS_EXTERNAL_DATA isc_logmodule_t dns_modules[] = {
	{ "dns/db",	 	0 },
	{ "dns/rbtdb", 		0 },
	{ "dns/rbtdb64", 	0 },
	{ "dns/rbt", 		0 },
	{ "dns/rdata", 		0 },
	{ "dns/master", 	0 },
	{ "dns/message", 	0 },
	{ "dns/cache", 		0 },
	{ "dns/config",		0 },
	{ "dns/resolver",	0 },
	{ "dns/zone",		0 },
	{ "dns/journal",	0 },
	{ "dns/adb",		0 },
	{ "dns/xfrin",		0 },
	{ "dns/xfrout",		0 },
	{ "dns/acl",		0 },
	{ "dns/validator",	0 },
	{ "dns/dispatch",	0 },
	{ "dns/request",	0 },
	{ "dns/masterdump",	0 },
	{ "dns/tsig",		0 },
	{ "dns/tkey",		0 },
	{ "dns/sdb",		0 },
	{ "dns/diff",		0 },
	{ "dns/hints",		0 },
	{ "dns/acache",		0 },
	{ "dns/dlz",		0 },
	{ "dns/dnssec",		0 },
	{ "dns/crypto",		0 },
	{ NULL, 		0 }
};

LIBDNS_EXTERNAL_DATA isc_log_t *dns_lctx = NULL;

void
dns_log_init(isc_log_t *lctx) {
	REQUIRE(lctx != NULL);

	isc_log_registercategories(lctx, dns_categories);
	isc_log_registermodules(lctx, dns_modules);
}

void
dns_log_setcontext(isc_log_t *lctx) {
	dns_lctx = lctx;
}