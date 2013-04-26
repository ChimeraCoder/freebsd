
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

MODULE_ID("$Id: frm_cursor.c,v 1.9 2004/12/11 22:01:03 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int pos_form_cursor(FORM * form)
|   
|   Description   :  Moves the form window cursor to the location required
|                    by the form driver to resume form processing. This may
|                    be needed after the application calls a curses library
|                    I/O routine that modifies the cursor position.
|
|   Return Values :  E_OK                      - Success
|                    E_SYSTEM_ERROR            - System error.
|                    E_BAD_ARGUMENT            - Invalid form pointer
|                    E_NOT_POSTED              - Form is not posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
pos_form_cursor(FORM *form)
{
  int res;

  T((T_CALLED("pos_form_cursor(%p)"), form));

  if (!form)
    res = E_BAD_ARGUMENT;
  else
    {
      if (!(form->status & _POSTED))
	res = E_NOT_POSTED;
      else
	res = _nc_Position_Form_Cursor(form);
    }
  RETURN(res);
}

/* frm_cursor.c ends here */