
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

MODULE_ID("$Id: fld_max.c,v 1.9 2004/12/11 21:51:54 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_max_field(FIELD *field, int maxgrow)
|   
|   Description   :  Set the maximum growth for a dynamic field. If maxgrow=0
|                    the field may grow to any possible size.
|
|   Return Values :  E_OK           - success
|                    E_BAD_ARGUMENT - invalid argument
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_max_field(FIELD *field, int maxgrow)
{
  T((T_CALLED("set_max_field(%p,%d)"), field, maxgrow));

  if (!field || (maxgrow < 0))
    RETURN(E_BAD_ARGUMENT);
  else
    {
      bool single_line_field = Single_Line_Field(field);

      if (maxgrow > 0)
	{
	  if ((single_line_field && (maxgrow < field->dcols)) ||
	      (!single_line_field && (maxgrow < field->drows)))
	    RETURN(E_BAD_ARGUMENT);
	}
      field->maxgrow = maxgrow;
      field->status &= ~_MAY_GROW;
      if (!(field->opts & O_STATIC))
	{
	  if ((maxgrow == 0) ||
	      (single_line_field && (field->dcols < maxgrow)) ||
	      (!single_line_field && (field->drows < maxgrow)))
	    field->status |= _MAY_GROW;
	}
    }
  RETURN(E_OK);
}

/* fld_max.c ends here */