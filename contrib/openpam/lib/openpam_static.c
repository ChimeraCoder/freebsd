
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

#ifdef OPENPAM_STATIC_MODULES

SET_DECLARE(openpam_static_modules, pam_module_t);

/*
 * OpenPAM internal
 *
 * Locate a statically linked module
 */

pam_module_t *
openpam_static(const char *path)
{
	pam_module_t **module;

	SET_FOREACH(module, openpam_static_modules) {
		if (strcmp((*module)->path, path) == 0)
			return (*module);
	}
	return (NULL);
}

#endif

/*
 * NOPARSE
 */