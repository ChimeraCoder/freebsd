
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

MODULE_ID("$Id: fld_user.c,v 1.15 2004/12/25 22:24:50 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_field_userptr(FIELD *field, void *usrptr)
|   
|   Description   :  Set the pointer that is reserved in any field to store
|                    application relevant informations
|
|   Return Values :  E_OK         - on success
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_field_userptr(FIELD *field, void *usrptr)
{
  T((T_CALLED("set_field_userptr(%p,%p)"), field, usrptr));

  Normalize_Field(field)->usrptr = usrptr;
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  void *field_userptr(const FIELD *field)
|   
|   Description   :  Return the pointer that is reserved in any field to
|                    store application relevant informations.
|
|   Return Values :  Value of pointer. If no such pointer has been set,
|                    NULL is returned
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(void *)
field_userptr(const FIELD *field)
{
  T((T_CALLED("field_userptr(%p)"), field));
  returnVoidPtr(Normalize_Field(field)->usrptr);
}

/* fld_user.c ends here */