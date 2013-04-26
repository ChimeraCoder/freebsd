
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

MODULE_ID("$Id: fld_page.c,v 1.9 2004/12/11 21:58:19 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_new_page(FIELD *field, bool new_page_flag)
|   
|   Description   :  Marks the field as the beginning of a new page of 
|                    the form.
|
|   Return Values :  E_OK         - success
|                    E_CONNECTED  - field is connected
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_new_page(FIELD *field, bool new_page_flag)
{
  T((T_CALLED("set_new_page(%p,%d)"), field, new_page_flag));

  Normalize_Field(field);
  if (field->form)
    RETURN(E_CONNECTED);

  if (new_page_flag)
    field->status |= _NEWPAGE;
  else
    field->status &= ~_NEWPAGE;

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  bool new_page(const FIELD *field)
|   
|   Description   :  Retrieve the info whether or not the field starts a
|                    new page on the form.
|
|   Return Values :  TRUE  - field starts a new page
|                    FALSE - field doesn't start a new page
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(bool)
new_page(const FIELD *field)
{
  T((T_CALLED("new_page(%p)"), field));

  returnBool((Normalize_Field(field)->status & _NEWPAGE) ? TRUE : FALSE);
}

/* fld_page.c ends here */