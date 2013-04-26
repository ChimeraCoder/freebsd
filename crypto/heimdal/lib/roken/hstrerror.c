
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

#include <config.h>

#ifndef HAVE_HSTRERROR

#if (defined(SunOS) && (SunOS >= 50))
#define hstrerror broken_proto
#endif
#include "roken.h"
#if (defined(SunOS) && (SunOS >= 50))
#undef hstrerror
#endif

#if !(defined(HAVE_H_ERRLIST) && defined(HAVE_H_NERR))
static const char *const h_errlist[] = {
    "Resolver Error 0 (no error)",
    "Unknown host",		/* 1 HOST_NOT_FOUND */
    "Host name lookup failure",	/* 2 TRY_AGAIN */
    "Unknown server error",	/* 3 NO_RECOVERY */
    "No address associated with name", /* 4 NO_ADDRESS */
};

static
const
int h_nerr = { sizeof h_errlist / sizeof h_errlist[0] };
#else

#if !HAVE_DECL_H_ERRLIST
extern const char *h_errlist[];
extern int h_nerr;
#endif

#endif

ROKEN_LIB_FUNCTION const char * ROKEN_LIB_CALL
hstrerror(int herr)
{
    if (0 <= herr && herr < h_nerr)
	return h_errlist[herr];
    else if(herr == -17)
	return "unknown error";
    else
	return "Error number out of range (hstrerror)";
}

#endif