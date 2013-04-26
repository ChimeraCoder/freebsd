
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>

#include <arm/at91/at91var.h>
#include <arm/at91/at91soc.h>

SET_DECLARE(at91_socs, const struct at91_soc);

struct at91_soc_data *
at91_match_soc(enum at91_soc_type type, enum at91_soc_subtype subtype)
{
	const struct at91_soc **socp;

	SET_FOREACH(socp, at91_socs) {
		if ((*socp)->soc_type != type)
			continue;
		if ((*socp)->soc_subtype != AT91_ST_ANY &&
		    (*socp)->soc_subtype != subtype)
			continue;
		return (*socp)->soc_data;
	}
	return NULL;
}