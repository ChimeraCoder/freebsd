
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

MODULE_ID("$Id: frm_post.c,v 1.9 2004/12/11 22:19:06 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int post_form(FORM * form)
|   
|   Description   :  Writes the form into its associated subwindow.
|
|   Return Values :  E_OK              - success
|                    E_BAD_ARGUMENT    - invalid form pointer
|                    E_POSTED          - form already posted
|                    E_NOT_CONNECTED   - no fields connected to form
|                    E_NO_ROOM         - form doesn't fit into subwindow
|                    E_SYSTEM_ERROR    - system error
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
post_form(FORM *form)
{
  WINDOW *formwin;
  int err;
  int page;

  T((T_CALLED("post_form(%p)"), form));

  if (!form)
    RETURN(E_BAD_ARGUMENT);

  if (form->status & _POSTED)
    RETURN(E_POSTED);

  if (!(form->field))
    RETURN(E_NOT_CONNECTED);

  formwin = Get_Form_Window(form);
  if ((form->cols > getmaxx(formwin)) || (form->rows > getmaxy(formwin)))
    RETURN(E_NO_ROOM);

  /* reset form->curpage to an invald value. This forces Set_Form_Page
     to do the page initialization which is required by post_form.
   */
  page = form->curpage;
  form->curpage = -1;
  if ((err = _nc_Set_Form_Page(form, page, form->current)) != E_OK)
    RETURN(err);

  form->status |= _POSTED;

  Call_Hook(form, forminit);
  Call_Hook(form, fieldinit);

  _nc_Refresh_Current_Field(form);
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int unpost_form(FORM * form)
|   
|   Description   :  Erase form from its associated subwindow.
|
|   Return Values :  E_OK            - success
|                    E_BAD_ARGUMENT  - invalid form pointer
|                    E_NOT_POSTED    - form isn't posted
|                    E_BAD_STATE     - called from a hook routine
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
unpost_form(FORM *form)
{
  T((T_CALLED("unpost_form(%p)"), form));

  if (!form)
    RETURN(E_BAD_ARGUMENT);

  if (!(form->status & _POSTED))
    RETURN(E_NOT_POSTED);

  if (form->status & _IN_DRIVER)
    RETURN(E_BAD_STATE);

  Call_Hook(form, fieldterm);
  Call_Hook(form, formterm);

  werase(Get_Form_Window(form));
  delwin(form->w);
  form->w = (WINDOW *)0;
  form->status &= ~_POSTED;
  RETURN(E_OK);
}

/* frm_post.c ends here */