
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

MODULE_ID("$Id: fld_just.c,v 1.11 2004/12/11 22:55:48 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_field_just(FIELD *field, int just)
|   
|   Description   :  Set the fields type of justification.
|
|   Return Values :  E_OK            - success
|                    E_BAD_ARGUMENT  - one of the arguments was incorrect
|                    E_SYSTEM_ERROR  - system error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_field_just(FIELD *field, int just)
{
  int res = E_BAD_ARGUMENT;

  T((T_CALLED("set_field_just(%p,%d)"), field, just));

  if ((just == NO_JUSTIFICATION) ||
      (just == JUSTIFY_LEFT) ||
      (just == JUSTIFY_CENTER) ||
      (just == JUSTIFY_RIGHT))
    {
      Normalize_Field(field);
      if (field->just != just)
	{
	  field->just = just;
	  res = _nc_Synchronize_Attributes(field);
	}
      else
	res = E_OK;
    }
  RETURN(res);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int field_just( const FIELD *field )
|   
|   Description   :  Retrieve the fields type of justification
|
|   Return Values :  The justification type.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
field_just(const FIELD *field)
{
  T((T_CALLED("field_just(%p)"), field));
  returnCode(Normalize_Field(field)->just);
}

/* fld_just.c ends here */