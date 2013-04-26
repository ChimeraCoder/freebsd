
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

#include <string.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * OpenPAM internal
 *
 * Locate an environment variable
 */

int
openpam_findenv(pam_handle_t *pamh,
	const char *name,
	size_t len)
{
	int i;

	ENTER();
	if (pamh == NULL)
		RETURNN(-1);
	for (i = 0; i < pamh->env_count; ++i)
		if (strncmp(pamh->env[i], name, len) == 0 &&
		    pamh->env[i][len] == '=')
			RETURNN(i);
	RETURNN(-1);
}

/*
 * NODOC
 */