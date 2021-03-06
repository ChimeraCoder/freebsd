
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

MODULE_ID("$Id: frm_sub.c,v 1.9 2004/12/11 22:13:39 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_form_sub(FORM *form, WINDOW *win)
|   
|   Description   :  Set the subwindow of the form to win. 
|
|   Return Values :  E_OK       - success
|                    E_POSTED   - form is posted
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_form_sub(FORM *form, WINDOW *win)
{
  T((T_CALLED("set_form_sub(%p,%p)"), form, win));

  if (form && (form->status & _POSTED))
    RETURN(E_POSTED);

  Normalize_Form(form)->sub = win;
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  WINDOW *form_sub(const FORM *)
|   
|   Description   :  Retrieve the window of the form.
|
|   Return Values :  The pointer to the Subwindow.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(WINDOW *)
form_sub(const FORM *form)
{
  const FORM *f;

  T((T_CALLED("form_sub(%p)"), form));

  f = Normalize_Form(form);
  returnWin(Get_Form_Window(f));
}

/* frm_sub.c ends here */