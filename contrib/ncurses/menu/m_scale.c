
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

/***************************************************************************
* Module m_scale                                                           *
* Menu scaling routine                                                     *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_scale.c,v 1.9 2004/12/11 23:29:34 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int scale_menu(const MENU *menu)
|   
|   Description   :  Returns the minimum window size necessary for the
|                    subwindow of menu.  
|
|   Return Values :  E_OK                  - success
|                    E_BAD_ARGUMENT        - invalid menu pointer
|                    E_NOT_CONNECTED       - no items are connected to menu
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
scale_menu(const MENU * menu, int *rows, int *cols)
{
  T((T_CALLED("scale_menu(%p,%p,%p)"), menu, rows, cols));

  if (!menu)
    RETURN(E_BAD_ARGUMENT);

  if (menu->items && *(menu->items))
    {
      if (rows)
	*rows = menu->height;
      if (cols)
	*cols = menu->width;
      RETURN(E_OK);
    }
  else
    RETURN(E_NOT_CONNECTED);
}

/* m_scale.c ends here */