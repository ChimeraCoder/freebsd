
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
* Module m_spacing                                                         *
* Routines to handle spacing between entries                               *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_spacing.c,v 1.17 2004/12/11 23:29:34 tom Exp $")

#define MAX_SPC_DESC ((TABSIZE) ? (TABSIZE) : 8)
#define MAX_SPC_COLS ((TABSIZE) ? (TABSIZE) : 8)
#define MAX_SPC_ROWS (3)

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  int set_menu_spacing(MENU *menu,int desc, int r, int c);
|
|   Description   :  Set the spacing between entries
|
|   Return Values :  E_OK                 - on success
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
set_menu_spacing(MENU * menu, int s_desc, int s_row, int s_col)
{
  MENU *m;			/* split for ATAC workaround */

  T((T_CALLED("set_menu_spacing(%p,%d,%d,%d)"), menu, s_desc, s_row, s_col));

  m = Normalize_Menu(menu);

  assert(m);
  if (m->status & _POSTED)
    RETURN(E_POSTED);

  if (((s_desc < 0) || (s_desc > MAX_SPC_DESC)) ||
      ((s_row < 0) || (s_row > MAX_SPC_ROWS)) ||
      ((s_col < 0) || (s_col > MAX_SPC_COLS)))
    RETURN(E_BAD_ARGUMENT);

  m->spc_desc = s_desc ? s_desc : 1;
  m->spc_rows = s_row ? s_row : 1;
  m->spc_cols = s_col ? s_col : 1;
  _nc_Calculate_Item_Length_and_Width(m);

  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu
|   Function      :  int menu_spacing (const MENU *,int *,int *,int *);
|
|   Description   :  Retrieve info about spacing between the entries
|
|   Return Values :  E_OK             - on success
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
menu_spacing(const MENU * menu, int *s_desc, int *s_row, int *s_col)
{
  const MENU *m;		/* split for ATAC workaround */

  T((T_CALLED("menu_spacing(%p,%p,%p,%p)"), menu, s_desc, s_row, s_col));

  m = Normalize_Menu(menu);

  assert(m);
  if (s_desc)
    *s_desc = m->spc_desc;
  if (s_row)
    *s_row = m->spc_rows;
  if (s_col)
    *s_col = m->spc_cols;

  RETURN(E_OK);
}

/* m_spacing.c ends here */