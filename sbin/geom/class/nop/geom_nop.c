
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

#include <stdio.h>
#include <stdint.h>
#include <libgeom.h>
#include <geom/nop/g_nop.h>

#include "core/geom.h"


uint32_t lib_version = G_LIB_VERSION;
uint32_t version = G_NOP_VERSION;

struct g_command class_commands[] = {
	{ "create", G_FLAG_VERBOSE | G_FLAG_LOADKLD, NULL,
	    {
		{ 'e', "error", "-1", G_TYPE_NUMBER },
		{ 'o', "offset", "0", G_TYPE_NUMBER },
		{ 'r', "rfailprob", "-1", G_TYPE_NUMBER },
		{ 's', "size", "0", G_TYPE_NUMBER },
		{ 'S', "secsize", "0", G_TYPE_NUMBER },
		{ 'w', "wfailprob", "-1", G_TYPE_NUMBER },
		G_OPT_SENTINEL
	    },
	    "[-v] [-e error] [-o offset] [-r rfailprob] [-s size] "
	    "[-S secsize] [-w wfailprob] dev ..."
	},
	{ "configure", G_FLAG_VERBOSE, NULL,
	    {
		{ 'e', "error", "-1", G_TYPE_NUMBER },
		{ 'r', "rfailprob", "-1", G_TYPE_NUMBER },
		{ 'w', "wfailprob", "-1", G_TYPE_NUMBER },
		G_OPT_SENTINEL
	    },
	    "[-v] [-e error] [-r rfailprob] [-w wfailprob] prov ..."
	},
	{ "destroy", G_FLAG_VERBOSE, NULL,
	    {
		{ 'f', "force", NULL, G_TYPE_BOOL },
		G_OPT_SENTINEL
	    },
	    "[-fv] prov ..."
	},
	{ "reset", G_FLAG_VERBOSE, NULL, G_NULL_OPTS,
	    "[-v] prov ..."
	},
	G_CMD_SENTINEL
};