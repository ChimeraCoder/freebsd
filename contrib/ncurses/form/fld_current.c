
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

MODULE_ID("$Id: fld_current.c,v 1.11 2004/12/25 22:40:13 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int set_current_field(FORM  * form,FIELD * field)
|
|   Description   :  Set the current field of the form to the specified one.
|
|   Return Values :  E_OK              - success
|                    E_BAD_ARGUMENT    - invalid form or field pointer
|                    E_REQUEST_DENIED  - field not selectable
|                    E_BAD_STATE       - called from a hook routine
|                    E_INVALID_FIELD   - current field can't be left
|                    E_SYSTEM_ERROR    - system error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_current_field(FORM *form, FIELD *field)
{
  int err = E_OK;

  T((T_CALLED("set_current_field(%p,%p)"), form, field));
  if (form == 0 || field == 0)
    {
      RETURN(E_BAD_ARGUMENT);
    }
  else if ((form != field->form) || Field_Is_Not_Selectable(field))
    {
      RETURN(E_REQUEST_DENIED);
    }
  else if ((form->status & _POSTED) == 0)
    {
      form->current = field;
      form->curpage = field->page;
    }
  else
    {
      if ((form->status & _IN_DRIVER) != 0)
	{
	  err = E_BAD_STATE;
	}
      else
	{
	  if (form->current != field)
	    {
	      if (!_nc_Internal_Validation(form))
		{
		  err = E_INVALID_FIELD;
		}
	      else
		{
		  Call_Hook(form, fieldterm);
		  if (field->page != form->curpage)
		    {
		      Call_Hook(form, formterm);
		      err = _nc_Set_Form_Page(form, (int)field->page, field);
		      Call_Hook(form, forminit);
		    }
		  else
		    {
		      err = _nc_Set_Current_Field(form, field);
		    }
		  Call_Hook(form, fieldinit);
		  (void)_nc_Refresh_Current_Field(form);
		}
	    }
	}
    }
  RETURN(err);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  FIELD *current_field(const FORM * form)
|
|   Description   :  Return the current field.
|
|   Return Values :  Pointer to the current field.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(FIELD *)
current_field(const FORM *form)
{
  T((T_CALLED("current_field(%p)"), form));
  returnField(Normalize_Form(form)->current);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int field_index(const FIELD * field)
|
|   Description   :  Return the index of the field in the field-array of
|                    the form.
|
|   Return Values :  >= 0   : field index
|                    -1     : fieldpointer invalid or field not connected
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
field_index(const FIELD *field)
{
  T((T_CALLED("field_index(%p)"), field));
  returnCode((field != 0 && field->form != 0) ? (int)field->index : -1);
}

/* fld_current.c ends here */