
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

MODULE_ID("$Id: fld_ftchoice.c,v 1.9 2004/12/11 21:44:57 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_fieldtype_choice(
|                          FIELDTYPE *typ,
|                          bool (* const next_choice)(FIELD *,const void *),
|                          bool (* const prev_choice)(FIELD *,const void *))
|
|   Description   :  Define implementation of enumeration requests.
|
|   Return Values :  E_OK           - success
|                    E_BAD_ARGUMENT - invalid arguments
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_fieldtype_choice(FIELDTYPE *typ,
		     bool (*const next_choice) (FIELD *, const void *),
		     bool (*const prev_choice) (FIELD *, const void *))
{
  T((T_CALLED("set_fieldtype_choice(%p,%p,%p)"), typ, next_choice, prev_choice));

  if (!typ || !next_choice || !prev_choice)
    RETURN(E_BAD_ARGUMENT);

  typ->status |= _HAS_CHOICE;
  typ->next = next_choice;
  typ->prev = prev_choice;
  RETURN(E_OK);
}

/* fld_ftchoice.c ends here */