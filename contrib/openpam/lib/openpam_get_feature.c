
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
#include <security/openpam.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Query the state of an optional feature.
 */

int
openpam_get_feature(int feature, int *onoff)
{

	ENTERF(feature);
	if (feature < 0 || feature >= OPENPAM_NUM_FEATURES)
		RETURNC(PAM_SYMBOL_ERR);
	*onoff = openpam_features[feature].onoff;
	RETURNC(PAM_SUCCESS);
}

/*
 * Error codes:
 *
 *	PAM_SYMBOL_ERR
 */

/**
 * EXPERIMENTAL
 *
 * The =openpam_get_feature function stores the current state of the
 * specified feature in the variable pointed to by its =onoff argument.
 *
 * The following features are recognized:
 *
 *	=OPENPAM_RESTRICT_SERVICE_NAME:
 *		Disallow path separators in service names.
 *		This feature is enabled by default.
 *		Disabling it allows the application to specify the path to
 *		the desired policy file directly.
 *
 *	=OPENPAM_VERIFY_POLICY_FILE:
 *		Verify the ownership and permissions of the policy file
 *		and the path leading up to it.
 *		This feature is enabled by default.
 *
 *	=OPENPAM_RESTRICT_MODULE_NAME:
 *		Disallow path separators in module names.
 *		This feature is disabled by default.
 *		Enabling it prevents the use of modules in non-standard
 *		locations.
 *
 *	=OPENPAM_VERIFY_MODULE_FILE:
 *		Verify the ownership and permissions of each loadable
 *		module and the path leading up to it.
 *		This feature is enabled by default.
 *
 *
 * >openpam_set_feature
 *
 * AUTHOR DES
 */