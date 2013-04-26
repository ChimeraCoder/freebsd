
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
* Module m_item_nam                                                        *
* Get menus item name and description                                      *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_item_nam.c,v 1.14 2004/12/25 21:41:54 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  char *item_name(const ITEM *item)
|   
|   Description   :  Return name of menu item
|
|   Return Values :  See above; returns NULL if item is invalid
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(const char *)
item_name(const ITEM * item)
{
  T((T_CALLED("item_name(%p)"), item));
  returnCPtr((item) ? item->name.str : (char *)0);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  char *item_description(const ITEM *item)
|   
|   Description   :  Returns description of item
|
|   Return Values :  See above; Returns NULL if item is invalid
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(const char *)
item_description(const ITEM * item)
{
  T((T_CALLED("item_description(%p)"), item));
  returnCPtr((item) ? item->description.str : (char *)0);
}

/* m_item_nam.c ends here */