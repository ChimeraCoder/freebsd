
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

#include <sys/param.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>

/*
 * XSSO 4.2.2
 * XSSO 6 page 70
 *
 * Service module implementation for pam_authenticate_secondary
 */

int
pam_sm_authenticate_secondary(pam_handle_t *pamh,
	char *target_username,
	char *target_module_type,
	char *target_authn_domain,
	char *target_supp_data,
	unsigned char *target_module_authtok,
	int flags,
	int argc,
	const char **argv)
{

	ENTER();
	RETURNC(PAM_SYSTEM_ERR);
}

/*
 * NODOC
 */