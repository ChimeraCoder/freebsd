
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
/* Last non-groff version: hpoint.c  1.1  84/10/08 */
/*
 * This file contains routines for manipulating the point data structures
 * for the gremlin picture editor.
 */

#include <stdlib.h>
#include "gprint.h"


/*
 * Return pointer to empty point list.
 */
POINT *
PTInit()
{
  return ((POINT *) NULL);
}


/*
 * This routine creates a new point with coordinates x and y and links it
 * into the pointlist.
 */
POINT *
PTMakePoint(double x,
	    double y,
	    POINT **pplist)
{
  register POINT *pt;

  if (Nullpoint(pt = *pplist)) {	/* empty list */
    *pplist = (POINT *) malloc(sizeof(POINT));
    pt = *pplist;
  } else {
    while (!Nullpoint(pt->nextpt))
      pt = pt->nextpt;
    pt->nextpt = (POINT *) malloc(sizeof(POINT));
    pt = pt->nextpt;
  }

  pt->x = x;
  pt->y = y;
  pt->nextpt = PTInit();
  return (pt);
}				/* end PTMakePoint */

/* EOF */