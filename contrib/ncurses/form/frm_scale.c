
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

MODULE_ID("$Id: frm_scale.c,v 1.9 2004/12/11 22:12:34 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int scale_form( const FORM *form, int *rows, int *cols )
|   
|   Description   :  Retrieve size of form
|
|   Return Values :  E_OK              - no error
|                    E_BAD_ARGUMENT    - invalid form pointer
|                    E_NOT_CONNECTED   - no fields connected to form
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
scale_form(const FORM *form, int *rows, int *cols)
{
  T((T_CALLED("scale_form(%p,%p,%p)"), form, rows, cols));

  if (!form)
    RETURN(E_BAD_ARGUMENT);

  if (!(form->field))
    RETURN(E_NOT_CONNECTED);

  if (rows)
    *rows = form->rows;
  if (cols)
    *cols = form->cols;

  RETURN(E_OK);
}

/* frm_scale.c ends here */