
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

/*
 * XSSO 4.2.1
 * XSSO 6 page 50
 *
 * Get valid matched identity in new domain
 */

int
pam_get_mapped_username(pam_handle_t *pamh,
	const char *src_username,
	const char *src_module_type,
	const char *src_authn_domain,
	const char *target_module_type,
	const char *target_authn_domain,
	char **target_module_username)
{

	ENTER();
	RETURNC(PAM_SYSTEM_ERR);
}

/*
 * NODOC
 */