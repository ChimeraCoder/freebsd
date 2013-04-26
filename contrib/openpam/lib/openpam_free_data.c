
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

#include <stdlib.h>
#include <string.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Generic cleanup function
 */

void
openpam_free_data(pam_handle_t *pamh,
	void *data,
	int status)
{

	ENTER();
	(void)pamh;
	(void)status;
	FREE(data);
	RETURNV();
}

/*
 * Error codes:
 */

/**
 * The =openpam_free_data function is a cleanup function suitable for
 * passing to =pam_set_data.
 * It simply releases the data by passing its =data argument to =free.
 */