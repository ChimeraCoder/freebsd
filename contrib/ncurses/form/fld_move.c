
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

/****************************************************************************
 *   Author:  Juergen Pfeifer, 1995,1997                                    *
 ****************************************************************************/

#include "form.priv.h"

MODULE_ID("$Id: fld_move.c,v 1.9 2004/12/11 21:52:44 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int move_field(FIELD *field,int frow, int fcol)
|   
|   Description   :  Moves the disconnected field to the new location in
|                    the forms subwindow.
|
|   Return Values :  E_OK            - success
|                    E_BAD_ARGUMENT  - invalid argument passed
|                    E_CONNECTED     - field is connected
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
move_field(FIELD *field, int frow, int fcol)
{
  T((T_CALLED("move_field(%p,%d,%d)"), field, frow, fcol));

  if (!field || (frow < 0) || (fcol < 0))
    RETURN(E_BAD_ARGUMENT);

  if (field->form)
    RETURN(E_CONNECTED);

  field->frow = frow;
  field->fcol = fcol;
  RETURN(E_OK);
}

/* fld_move.c ends here */