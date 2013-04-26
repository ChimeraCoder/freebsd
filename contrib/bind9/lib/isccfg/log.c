
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

/* $Id: log.c,v 1.11 2007/06/19 23:47:22 tbox Exp $ */

/*! \file */

#include <config.h>

#include <isc/util.h>

#include <isccfg/log.h>

/*%
 * When adding a new category, be sure to add the appropriate
 * \#define to <isccfg/log.h>.
 */
LIBISCCFG_EXTERNAL_DATA isc_logcategory_t cfg_categories[] = {
	{ "config", 	0 },
	{ NULL, 	0 }
};

/*%
 * When adding a new module, be sure to add the appropriate
 * \#define to <isccfg/log.h>.
 */
LIBISCCFG_EXTERNAL_DATA isc_logmodule_t cfg_modules[] = {
	{ "isccfg/parser",	0 },
	{ NULL, 		0 }
};

void
cfg_log_init(isc_log_t *lctx) {
	REQUIRE(lctx != NULL);

	isc_log_registercategories(lctx, cfg_categories);
	isc_log_registermodules(lctx, cfg_modules);
}