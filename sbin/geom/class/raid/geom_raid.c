
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
#include <errno.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <libgeom.h>
#include <geom/raid/g_raid.h>
#include <core/geom.h>
#include <misc/subr.h>

uint32_t lib_version = G_LIB_VERSION;
uint32_t version = G_RAID_VERSION;

struct g_command class_commands[] = {
	{ "label", G_FLAG_VERBOSE, NULL,
	    {
		{ 'f', "force", NULL, G_TYPE_BOOL },
		{ 'o', "fmtopt", G_VAL_OPTIONAL, G_TYPE_STRING },
		{ 'S', "size", G_VAL_OPTIONAL, G_TYPE_NUMBER },
		{ 's', "strip", G_VAL_OPTIONAL, G_TYPE_NUMBER },
		G_OPT_SENTINEL
	    },
	    "[-fv] [-o fmtopt] [-S size] [-s stripsize] format label level prov ..."
	},
	{ "add", G_FLAG_VERBOSE, NULL,
	    {
		{ 'f', "force", NULL, G_TYPE_BOOL },
		{ 'S', "size", G_VAL_OPTIONAL, G_TYPE_NUMBER },
		{ 's', "strip", G_VAL_OPTIONAL, G_TYPE_NUMBER },
		G_OPT_SENTINEL
	    },
	    "[-fv] [-S size] [-s stripsize] name label level"
	},
	{ "delete", G_FLAG_VERBOSE, NULL,
	    {
		{ 'f', "force", NULL, G_TYPE_BOOL },
		G_OPT_SENTINEL
	    },
	    "[-fv] name [label|num]"
	},
	{ "insert", G_FLAG_VERBOSE, NULL, G_NULL_OPTS,
	    "[-v] name prov ..."
	},
	{ "remove", G_FLAG_VERBOSE, NULL, G_NULL_OPTS,
	    "[-v] name prov ..."
	},
	{ "fail", G_FLAG_VERBOSE, NULL, G_NULL_OPTS,
	    "[-v] name prov ..."
	},
	{ "stop", G_FLAG_VERBOSE, NULL,
	    {
		{ 'f', "force", NULL, G_TYPE_BOOL },
		G_OPT_SENTINEL
	    },
	    "[-fv] name"
	},
	G_CMD_SENTINEL
};