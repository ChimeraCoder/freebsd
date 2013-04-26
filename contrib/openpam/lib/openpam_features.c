
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <security/pam_appl.h>

#include "openpam_impl.h"

#define STRUCT_OPENPAM_FEATURE(name, descr, dflt)	\
	[OPENPAM_##name] = {				\
		"OPENPAM_" #name,			\
		descr,					\
		dflt					\
	}

struct openpam_feature openpam_features[OPENPAM_NUM_FEATURES] = {
	STRUCT_OPENPAM_FEATURE(
	    RESTRICT_SERVICE_NAME,
	    "Disallow path separators in service names",
	    1
	),
	STRUCT_OPENPAM_FEATURE(
	    VERIFY_POLICY_FILE,
	    "Verify ownership and permissions of policy files",
	    1
	),
	STRUCT_OPENPAM_FEATURE(
	    RESTRICT_MODULE_NAME,
	    "Disallow path separators in module names",
	    0
	),
	STRUCT_OPENPAM_FEATURE(
	    VERIFY_MODULE_FILE,
	    "Verify ownership and permissions of module files",
	    1
	),
};