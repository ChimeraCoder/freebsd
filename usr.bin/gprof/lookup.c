
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)lookup.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "gprof.h"

    /*
     *	look up an address in a sorted-by-address namelist
     *	    this deals with misses by mapping them to the next lower
     *	    entry point.
     */
nltype *
nllookup(unsigned long address)
{
    register long	low;
    register long	middle;
    register long	high;
#   ifdef DEBUG
	register int	probes;

	probes = 0;
#   endif /* DEBUG */
    for ( low = 0 , high = nname - 1 ; low != high ; ) {
#	ifdef DEBUG
	    probes += 1;
#	endif /* DEBUG */
	middle = ( high + low ) >> 1;
	if ( nl[ middle ].value <= address && nl[ middle+1 ].value > address ) {
#	    ifdef DEBUG
		if ( debug & LOOKUPDEBUG ) {
		    printf( "[nllookup] %d (%d) probes\n" , probes , nname-1 );
		}
#	    endif /* DEBUG */
#if defined(__arm__)
	if (nl[middle].name[0] == '$' &&
	    nl[middle-1].value == nl[middle].value)
		middle--;
#endif

	    return &nl[ middle ];
	}
	if ( nl[ middle ].value > address ) {
	    high = middle;
	} else {
	    low = middle + 1;
	}
    }
#   ifdef DEBUG
	if ( debug & LOOKUPDEBUG ) {
	    fprintf( stderr , "[nllookup] (%d) binary search fails\n" ,
		nname-1 );
	}
#   endif /* DEBUG */
    return 0;
}

arctype *
arclookup(nltype *parentp, nltype *childp)
{
    arctype	*arcp;

    if ( parentp == 0 || childp == 0 ) {
	fprintf( stderr, "[arclookup] parentp == 0 || childp == 0\n" );
	return 0;
    }
#   ifdef DEBUG
	if ( debug & LOOKUPDEBUG ) {
	    printf( "[arclookup] parent %s child %s\n" ,
		    parentp -> name , childp -> name );
	}
#   endif /* DEBUG */
    for ( arcp = parentp -> children ; arcp ; arcp = arcp -> arc_childlist ) {
#	ifdef DEBUG
	    if ( debug & LOOKUPDEBUG ) {
		printf( "[arclookup]\t arc_parent %s arc_child %s\n" ,
			arcp -> arc_parentp -> name ,
			arcp -> arc_childp -> name );
	    }
#	endif /* DEBUG */
	if ( arcp -> arc_childp == childp ) {
	    return arcp;
	}
    }
    return 0;
}