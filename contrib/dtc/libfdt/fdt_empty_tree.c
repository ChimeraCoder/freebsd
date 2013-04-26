
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
#include "libfdt_env.h"

#include <fdt.h>
#include <libfdt.h>

#include "libfdt_internal.h"

int fdt_create_empty_tree(void *buf, int bufsize)
{
	int err;

	err = fdt_create(buf, bufsize);
	if (err)
		return err;

	err = fdt_finish_reservemap(buf);
	if (err)
		return err;

	err = fdt_begin_node(buf, "");
	if (err)
		return err;

	err =  fdt_end_node(buf);
	if (err)
		return err;

	err = fdt_finish(buf);
	if (err)
		return err;

	return fdt_open_into(buf, buf, bufsize);
}