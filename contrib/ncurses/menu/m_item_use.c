
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
* Module m_item_use                                                        *
* Associate application data with menu items                               *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_item_use.c,v 1.16 2004/12/25 21:33:31 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_item_userptr(ITEM *item, void *userptr)
|   
|   Description   :  Set the pointer that is reserved in any item to store
|                    application relevant informations.  
|
|   Return Values :  E_OK               - success
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_item_userptr(ITEM * item, void *userptr)
{
  T((T_CALLED("set_item_userptr(%p,%p)"), item, userptr));
  Normalize_Item(item)->userptr = userptr;
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  void *item_userptr(const ITEM *item)
|   
|   Description   :  Return the pointer that is reserved in any item to store
|                    application relevant informations.
|
|   Return Values :  Value of the pointer. If no such pointer has been set,
|                    NULL is returned.
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(void *)
item_userptr(const ITEM * item)
{
  T((T_CALLED("item_userptr(%p)"), item));
  returnVoidPtr(Normalize_Item(item)->userptr);
}

/* m_item_use.c */