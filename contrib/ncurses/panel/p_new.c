
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
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1995                    *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Juergen Pfeifer                         1997-1999               *
 *     and: Thomas E. Dickey                        2000-on                 *
 ****************************************************************************/

/* p_new.c
 * Creation of a new panel 
 */
#include "panel.priv.h"

MODULE_ID("$Id: p_new.c,v 1.10 2008/08/04 18:25:48 tom Exp $")

#ifdef TRACE
static char *stdscr_id;
static char *new_id;
#endif

/*+-------------------------------------------------------------------------
  Get root (i.e. stdscr's) panel.
  Establish the pseudo panel for stdscr if necessary.
--------------------------------------------------------------------------*/
static PANEL *
root_panel(void)
{
  if (_nc_stdscr_pseudo_panel == (PANEL *) 0)
    {

      assert(stdscr && !_nc_bottom_panel && !_nc_top_panel);
#if NO_LEAKS
      _nc_panelhook()->destroy = del_panel;
#endif
      _nc_stdscr_pseudo_panel = (PANEL *) malloc(sizeof(PANEL));
      if (_nc_stdscr_pseudo_panel != 0)
	{
	  PANEL *pan = _nc_stdscr_pseudo_panel;
	  WINDOW *win = stdscr;

	  pan->win = win;
	  pan->below = (PANEL *) 0;
	  pan->above = (PANEL *) 0;
#ifdef TRACE
	  if (!stdscr_id)
	    stdscr_id = strdup("stdscr");
	  pan->user = stdscr_id;
#else
	  pan->user = (void *)0;
#endif
	  _nc_bottom_panel = _nc_top_panel = pan;
	}
    }
  return _nc_stdscr_pseudo_panel;
}

NCURSES_EXPORT(PANEL *)
new_panel(WINDOW *win)
{
  PANEL *pan = (PANEL *) 0;

  T((T_CALLED("new_panel(%p)"), win));

  if (!win)
    returnPanel(pan);

  if (!_nc_stdscr_pseudo_panel)
    (void)root_panel();
  assert(_nc_stdscr_pseudo_panel);

  if (!(win->_flags & _ISPAD) && (pan = (PANEL *) malloc(sizeof(PANEL))))
    {
      pan->win = win;
      pan->above = (PANEL *) 0;
      pan->below = (PANEL *) 0;
#ifdef TRACE
      if (!new_id)
	new_id = strdup("new");
      pan->user = new_id;
#else
      pan->user = (char *)0;
#endif
      (void)show_panel(pan);
    }
  returnPanel(pan);
}