
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
 * Enable or disable an optional feature.
 */

int
openpam_set_feature(int feature, int onoff)
{

	ENTERF(feature);
	if (feature < 0 || feature >= OPENPAM_NUM_FEATURES)
		RETURNC(PAM_SYMBOL_ERR);
	openpam_features[feature].onoff = onoff;
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
 * The =openpam_set_feature function sets the state of the specified
 * feature to the value specified by the =onoff argument.
 * See =openpam_get_feature for a list of recognized features.
 *
 * >openpam_get_feature
 *
 * AUTHOR DES
 */