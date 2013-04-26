
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
* Module m_hook                                                            *
* Assign application specific routines for automatic invocation by menus   *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_hook.c,v 1.14 2004/12/25 21:39:52 tom Exp $")

/* "Template" macro to generate function to set application specific hook */
#define GEN_HOOK_SET_FUNCTION( typ, name ) \
NCURSES_IMPEXP int NCURSES_API set_ ## typ ## _ ## name (MENU *menu, Menu_Hook func )\
{\
   T((T_CALLED("set_" #typ "_" #name "(%p,%p)"), menu, func));\
   (Normalize_Menu(menu) -> typ ## name = func );\
   RETURN(E_OK);\
}

/* "Template" macro to generate function to get application specific hook */
#define GEN_HOOK_GET_FUNCTION( typ, name ) \
NCURSES_IMPEXP Menu_Hook NCURSES_API typ ## _ ## name ( const MENU *menu )\
{\
   T((T_CALLED(#typ "_" #name "(%p)"), menu));\
   returnMenuHook(Normalize_Menu(menu) -> typ ## name);\
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_init(MENU *menu, void (*f)(MENU *))
|   
|   Description   :  Set user-exit which is called when menu is posted
|                    or just after the top row changes.
|
|   Return Values :  E_OK               - success
+--------------------------------------------------------------------------*/
GEN_HOOK_SET_FUNCTION(menu, init)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  void (*)(MENU *) menu_init(const MENU *menu)
|   
|   Description   :  Return address of user-exit function which is called
|                    when a menu is posted or just after the top row 
|                    changes.
|
|   Return Values :  Menu init function address or NULL
+--------------------------------------------------------------------------*/
GEN_HOOK_GET_FUNCTION(menu, init)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_menu_term (MENU *menu, void (*f)(MENU *))
|   
|   Description   :  Set user-exit which is called when menu is unposted
|                    or just before the top row changes.
|
|   Return Values :  E_OK               - success
+--------------------------------------------------------------------------*/
GEN_HOOK_SET_FUNCTION(menu, term)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  void (*)(MENU *) menu_term(const MENU *menu)
|   
|   Description   :  Return address of user-exit function which is called
|                    when a menu is unposted or just before the top row 
|                    changes.
|
|   Return Values :  Menu finalization function address or NULL
+--------------------------------------------------------------------------*/
GEN_HOOK_GET_FUNCTION(menu, term)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_item_init (MENU *menu, void (*f)(MENU *))
|   
|   Description   :  Set user-exit which is called when menu is posted
|                    or just after the current item changes.
|
|   Return Values :  E_OK               - success
+--------------------------------------------------------------------------*/
GEN_HOOK_SET_FUNCTION(item, init)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  void (*)(MENU *) item_init (const MENU *menu)
|   
|   Description   :  Return address of user-exit function which is called
|                    when a menu is posted or just after the current item 
|                    changes.
|
|   Return Values :  Item init function address or NULL
+--------------------------------------------------------------------------*/
GEN_HOOK_GET_FUNCTION(item, init)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int set_item_term (MENU *menu, void (*f)(MENU *))
|   
|   Description   :  Set user-exit which is called when menu is unposted
|                    or just before the current item changes.
|
|   Return Values :  E_OK               - success
+--------------------------------------------------------------------------*/
GEN_HOOK_SET_FUNCTION(item, term)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  void (*)(MENU *) item_init (const MENU *menu)
|   
|   Description   :  Return address of user-exit function which is called
|                    when a menu is unposted or just before the current item 
|                    changes.
|
|   Return Values :  Item finalization function address or NULL
+--------------------------------------------------------------------------*/
GEN_HOOK_GET_FUNCTION(item, term)

/* m_hook.c ends here */