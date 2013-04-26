
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

MODULE_ID("$Id: frm_page.c,v 1.10 2004/12/11 22:08:21 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_form_page(FORM * form,int  page)
|   
|   Description   :  Set the page number of the form.
|
|   Return Values :  E_OK              - success
|                    E_BAD_ARGUMENT    - invalid form pointer or page number
|                    E_BAD_STATE       - called from a hook routine
|                    E_INVALID_FIELD   - current field can't be left
|                    E_SYSTEM_ERROR    - system error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_form_page(FORM *form, int page)
{
  int err = E_OK;

  T((T_CALLED("set_form_page(%p,%d)"), form, page));

  if (!form || (page < 0) || (page >= form->maxpage))
    RETURN(E_BAD_ARGUMENT);

  if (!(form->status & _POSTED))
    {
      form->curpage = page;
      form->current = _nc_First_Active_Field(form);
    }
  else
    {
      if (form->status & _IN_DRIVER)
	err = E_BAD_STATE;
      else
	{
	  if (form->curpage != page)
	    {
	      if (!_nc_Internal_Validation(form))
		err = E_INVALID_FIELD;
	      else
		{
		  Call_Hook(form, fieldterm);
		  Call_Hook(form, formterm);
		  err = _nc_Set_Form_Page(form, page, (FIELD *)0);
		  Call_Hook(form, forminit);
		  Call_Hook(form, fieldinit);
		  _nc_Refresh_Current_Field(form);
		}
	    }
	}
    }
  RETURN(err);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int form_page(const FORM * form)
|   
|   Description   :  Return the current page of the form.
|
|   Return Values :  >= 0  : current page number
|                    -1    : invalid form pointer
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
form_page(const FORM *form)
{
  T((T_CALLED("form_page(%p)"), form));

  returnCode(Normalize_Form(form)->curpage);
}

/* frm_page.c ends here */