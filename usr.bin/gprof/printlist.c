
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
static char sccsid[] = "@(#)printlist.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <string.h>

#include "gprof.h"

    /*
     *	these are the lists of names:
     *	there is the list head and then the listname
     *	is a pointer to the list head
     *	(for ease of passing to stringlist functions).
     */
struct stringlist	kfromhead = { 0 , 0 };
struct stringlist	*kfromlist = &kfromhead;
struct stringlist	ktohead = { 0 , 0 };
struct stringlist	*ktolist = &ktohead;
struct stringlist	fhead = { 0 , 0 };
struct stringlist	*flist = &fhead;
struct stringlist	Fhead = { 0 , 0 };
struct stringlist	*Flist = &Fhead;
struct stringlist	ehead = { 0 , 0 };
struct stringlist	*elist = &ehead;
struct stringlist	Ehead = { 0 , 0 };
struct stringlist	*Elist = &Ehead;

void
addlist(struct stringlist *listp, char *funcname)
{
    struct stringlist	*slp;

    slp = (struct stringlist *) malloc( sizeof(struct stringlist));
    if ( slp == (struct stringlist *) 0 )
	errx( 1 , "no room for printlist");
    slp -> next = listp -> next;
    slp -> string = funcname;
    listp -> next = slp;
}

bool
onlist(struct stringlist *listp, const char *funcname)
{
    struct stringlist	*slp;

    for ( slp = listp -> next ; slp ; slp = slp -> next ) {
	if ( ! strcmp( slp -> string , funcname ) ) {
	    return TRUE;
	}
	if ( funcname[0] == '_' && ! strcmp( slp -> string , &funcname[1] ) ) {
	    return TRUE;
	}
    }
    return FALSE;
}