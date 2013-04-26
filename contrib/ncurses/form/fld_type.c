
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

MODULE_ID("$Id: fld_type.c,v 1.15 2004/12/25 22:24:10 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_field_type(FIELD *field, FIELDTYPE *type,...)
|   
|   Description   :  Associate the specified fieldtype with the field.
|                    Certain field types take additional arguments. Look
|                    at the spec of the field types !
|
|   Return Values :  E_OK           - success
|                    E_SYSTEM_ERROR - system error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_field_type(FIELD *field, FIELDTYPE *type,...)
{
  va_list ap;
  int res = E_SYSTEM_ERROR;
  int err = 0;

  T((T_CALLED("set_field_type(%p,%p)"), field, type));

  va_start(ap, type);

  Normalize_Field(field);
  _nc_Free_Type(field);

  field->type = type;
  field->arg = (void *)_nc_Make_Argument(field->type, &ap, &err);

  if (err)
    {
      _nc_Free_Argument(field->type, (TypeArgument *)(field->arg));
      field->type = (FIELDTYPE *)0;
      field->arg = (void *)0;
    }
  else
    {
      res = E_OK;
      if (field->type)
	field->type->ref++;
    }

  va_end(ap);
  RETURN(res);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  FIELDTYPE *field_type(const FIELD *field)
|   
|   Description   :  Retrieve the associated fieldtype for this field.
|
|   Return Values :  Pointer to fieldtype of NULL if none is defined.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(FIELDTYPE *)
field_type(const FIELD *field)
{
  T((T_CALLED("field_type(%p)"), field));
  returnFieldType(Normalize_Field(field)->type);
}

/* fld_type.c ends here */