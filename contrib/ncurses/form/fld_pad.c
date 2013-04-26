
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

MODULE_ID("$Id: fld_pad.c,v 1.9 2004/12/11 21:56:49 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_field_pad(FIELD *field, int ch)
|   
|   Description   :  Set the pad character used to fill the field. This must
|                    be a printable character.
|
|   Return Values :  E_OK           - success
|                    E_BAD_ARGUMENT - invalid field pointer or pad character
|                    E_SYSTEM_ERROR - system error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_field_pad(FIELD *field, int ch)
{
  int res = E_BAD_ARGUMENT;

  T((T_CALLED("set_field_pad(%p,%d)"), field, ch));

  Normalize_Field(field);
  if (isprint(UChar(ch)))
    {
      if (field->pad != ch)
	{
	  field->pad = ch;
	  res = _nc_Synchronize_Attributes(field);
	}
      else
	res = E_OK;
    }
  RETURN(res);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int field_pad(const FIELD *field)
|   
|   Description   :  Retrieve the fields pad character.
|
|   Return Values :  The pad character.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
field_pad(const FIELD *field)
{
  T((T_CALLED("field_pad(%p)"), field));

  returnCode(Normalize_Field(field)->pad);
}

/* fld_pad.c ends here */