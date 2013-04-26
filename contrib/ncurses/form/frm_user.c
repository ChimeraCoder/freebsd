
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

MODULE_ID("$Id: frm_user.c,v 1.14 2004/12/25 22:37:56 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_form_userptr(FORM *form, void *usrptr)
|   
|   Description   :  Set the pointer that is reserved in any form to store
|                    application relevant informations
|
|   Return Values :  E_OK         - on success
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_form_userptr(FORM *form, void *usrptr)
{
  T((T_CALLED("set_form_userptr(%p,%p)"), form, usrptr));

  Normalize_Form(form)->usrptr = usrptr;
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  void *form_userptr(const FORM *form)
|   
|   Description   :  Return the pointer that is reserved in any form to
|                    store application relevant informations.
|
|   Return Values :  Value of pointer. If no such pointer has been set,
|                    NULL is returned
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(void *)
form_userptr(const FORM *form)
{
  T((T_CALLED("form_userptr(%p)"), form));
  returnVoidPtr(Normalize_Form(form)->usrptr);
}

/* frm_user.c ends here */