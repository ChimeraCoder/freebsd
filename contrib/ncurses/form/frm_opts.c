
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

MODULE_ID("$Id: frm_opts.c,v 1.14 2005/04/16 16:59:18 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_form_opts(FORM *form, Form_Options opts)
|   
|   Description   :  Turns on the named options and turns off all the
|                    remaining options for that form.
|
|   Return Values :  E_OK              - success
|                    E_BAD_ARGUMENT    - invalid options
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_form_opts(FORM *form, Form_Options opts)
{
  T((T_CALLED("set_form_opts(%p,%d)"), form, opts));

  opts &= ALL_FORM_OPTS;
  if (opts & ~ALL_FORM_OPTS)
    RETURN(E_BAD_ARGUMENT);
  else
    {
      Normalize_Form(form)->opts = opts;
      RETURN(E_OK);
    }
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  Form_Options form_opts(const FORM *)
|   
|   Description   :  Retrieves the current form options.
|
|   Return Values :  The option flags.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(Form_Options)
form_opts(const FORM *form)
{
  T((T_CALLED("form_opts(%p)"), form));
  returnCode((int)(Normalize_Form(form)->opts & ALL_FORM_OPTS));
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int form_opts_on(FORM *form, Form_Options opts)
|   
|   Description   :  Turns on the named options; no other options are 
|                    changed.
|
|   Return Values :  E_OK            - success 
|                    E_BAD_ARGUMENT  - invalid options
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
form_opts_on(FORM *form, Form_Options opts)
{
  T((T_CALLED("form_opts_on(%p,%d)"), form, opts));

  opts &= ALL_FORM_OPTS;
  if (opts & ~ALL_FORM_OPTS)
    RETURN(E_BAD_ARGUMENT);
  else
    {
      Normalize_Form(form)->opts |= opts;
      RETURN(E_OK);
    }
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int form_opts_off(FORM *form, Form_Options opts)
|   
|   Description   :  Turns off the named options; no other options are 
|                    changed.
|
|   Return Values :  E_OK            - success 
|                    E_BAD_ARGUMENT  - invalid options
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
form_opts_off(FORM *form, Form_Options opts)
{
  T((T_CALLED("form_opts_off(%p,%d)"), form, opts));

  opts &= ALL_FORM_OPTS;
  if (opts & ~ALL_FORM_OPTS)
    RETURN(E_BAD_ARGUMENT);
  else
    {
      Normalize_Form(form)->opts &= ~opts;
      RETURN(E_OK);
    }
}

/* frm_opts.c ends here */