
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

MODULE_ID("$Id: fld_stat.c,v 1.11 2004/12/11 22:28:00 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_field_status(FIELD *field, bool status)
|   
|   Description   :  Set or clear the 'changed' indication flag for that
|                    fields primary buffer.
|
|   Return Values :  E_OK            - success
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_field_status(FIELD *field, bool status)
{
  T((T_CALLED("set_field_status(%p,%d)"), field, status));

  Normalize_Field(field);

  if (status)
    field->status |= _CHANGED;
  else
    field->status &= ~_CHANGED;

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  bool field_status(const FIELD *field)
|   
|   Description   :  Retrieve the value of the 'changed' indication flag
|                    for that fields primary buffer. 
|
|   Return Values :  TRUE  - buffer has been changed
|                    FALSE - buffer has not been changed
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(bool)
field_status(const FIELD *field)
{
  T((T_CALLED("field_status(%p)"), field));

  returnBool((Normalize_Field(field)->status & _CHANGED) ? TRUE : FALSE);
}

/* fld_stat.c ends here */