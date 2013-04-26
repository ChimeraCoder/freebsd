
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

MODULE_ID("$Id: fld_opts.c,v 1.11 2004/12/11 21:55:46 tom Exp $")

/*----------------------------------------------------------------------------
  Field-Options manipulation routines
  --------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_field_opts(FIELD *field, Field_Options opts)
|   
|   Description   :  Turns on the named options for this field and turns
|                    off all the remaining options.
|
|   Return Values :  E_OK            - success
|                    E_CURRENT       - the field is the current field
|                    E_BAD_ARGUMENT  - invalid options
|                    E_SYSTEM_ERROR  - system error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_field_opts(FIELD *field, Field_Options opts)
{
  int res = E_BAD_ARGUMENT;

  T((T_CALLED("set_field_opts(%p,%d)"), field, opts));

  opts &= ALL_FIELD_OPTS;
  if (!(opts & ~ALL_FIELD_OPTS))
    res = _nc_Synchronize_Options(Normalize_Field(field), opts);
  RETURN(res);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  Field_Options field_opts(const FIELD *field)
|   
|   Description   :  Retrieve the fields options.
|
|   Return Values :  The options.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(Field_Options)
field_opts(const FIELD *field)
{
  T((T_CALLED("field_opts(%p)"), field));

  returnCode(ALL_FIELD_OPTS & Normalize_Field(field)->opts);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int field_opts_on(FIELD *field, Field_Options opts)
|   
|   Description   :  Turns on the named options for this field and all the 
|                    remaining options are unchanged.
|
|   Return Values :  E_OK            - success
|                    E_CURRENT       - the field is the current field
|                    E_BAD_ARGUMENT  - invalid options
|                    E_SYSTEM_ERROR  - system error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
field_opts_on(FIELD *field, Field_Options opts)
{
  int res = E_BAD_ARGUMENT;

  T((T_CALLED("field_opts_on(%p,%d)"), field, opts));

  opts &= ALL_FIELD_OPTS;
  if (!(opts & ~ALL_FIELD_OPTS))
    {
      Normalize_Field(field);
      res = _nc_Synchronize_Options(field, field->opts | opts);
    }
  RETURN(res);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int field_opts_off(FIELD *field, Field_Options opts)
|   
|   Description   :  Turns off the named options for this field and all the 
|                    remaining options are unchanged.
|
|   Return Values :  E_OK            - success
|                    E_CURRENT       - the field is the current field
|                    E_BAD_ARGUMENT  - invalid options
|                    E_SYSTEM_ERROR  - system error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
field_opts_off(FIELD *field, Field_Options opts)
{
  int res = E_BAD_ARGUMENT;

  T((T_CALLED("field_opts_off(%p,%d)"), field, opts));

  opts &= ALL_FIELD_OPTS;
  if (!(opts & ~ALL_FIELD_OPTS))
    {
      Normalize_Field(field);
      res = _nc_Synchronize_Options(field, field->opts & ~opts);
    }
  RETURN(res);
}

/* fld_opts.c ends here */