
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

/*
 * Abstract:
 *	Defines string to decode cl_status_t return values.
 *
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <complib/cl_types.h>

/* Status values above converted to text for easier printing. */
const char *cl_status_text[] = {
	"CL_SUCCESS",
	"CL_ERROR",
	"CL_INVALID_STATE",
	"CL_INVALID_OPERATION",
	"CL_INVALID_SETTING",
	"CL_INVALID_PARAMETER",
	"CL_INSUFFICIENT_RESOURCES",
	"CL_INSUFFICIENT_MEMORY",
	"CL_INVALID_PERMISSION",
	"CL_COMPLETED",
	"CL_NOT_DONE",
	"CL_PENDING",
	"CL_TIMEOUT",
	"CL_CANCELED",
	"CL_REJECT",
	"CL_OVERRUN",
	"CL_NOT_FOUND",
	"CL_UNAVAILABLE",
	"CL_BUSY",
	"CL_DISCONNECT",
	"CL_DUPLICATE"
};