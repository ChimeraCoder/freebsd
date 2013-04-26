
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
 ****************************************************************************/

/* panel.c -- implementation of panels library, some core routines */
#include "panel.priv.h"

MODULE_ID("$Id: panel.c,v 1.23 2005/02/19 18:04:31 tom Exp $")

/*+-------------------------------------------------------------------------
	_nc_retrace_panel (pan)
--------------------------------------------------------------------------*/
#ifdef TRACE
NCURSES_EXPORT(PANEL *)
_nc_retrace_panel(PANEL * pan)
{
  T((T_RETURN("%p"), pan));
  return pan;
}
#endif

/*+-------------------------------------------------------------------------
	_nc_my_visbuf(ptr)
--------------------------------------------------------------------------*/
#ifdef TRACE
#ifndef TRACE_TXT
NCURSES_EXPORT(const char *)
_nc_my_visbuf(const void *ptr)
{
  char temp[32];

  if (ptr != 0)
    sprintf(temp, "ptr:%p", ptr);
  else
    strcpy(temp, "<null>");
  return _nc_visbuf(temp);
}
#endif
#endif

/*+-------------------------------------------------------------------------
	dPanel(text,pan)
--------------------------------------------------------------------------*/
#ifdef TRACE
NCURSES_EXPORT(void)
_nc_dPanel(const char *text, const PANEL * pan)
{
  _tracef("%s id=%s b=%s a=%s y=%d x=%d",
	  text, USER_PTR(pan->user),
	  (pan->below) ? USER_PTR(pan->below->user) : "--",
	  (pan->above) ? USER_PTR(pan->above->user) : "--",
	  PSTARTY(pan), PSTARTX(pan));
}
#endif

/*+-------------------------------------------------------------------------
	dStack(fmt,num,pan)
--------------------------------------------------------------------------*/
#ifdef TRACE
NCURSES_EXPORT(void)
_nc_dStack(const char *fmt, int num, const PANEL * pan)
{
  char s80[80];

  sprintf(s80, fmt, num, pan);
  _tracef("%s b=%s t=%s", s80,
	  (_nc_bottom_panel) ? USER_PTR(_nc_bottom_panel->user) : "--",
	  (_nc_top_panel) ? USER_PTR(_nc_top_panel->user) : "--");
  if (pan)
    _tracef("pan id=%s", USER_PTR(pan->user));
  pan = _nc_bottom_panel;
  while (pan)
    {
      dPanel("stk", pan);
      pan = pan->above;
    }
}
#endif

/*+-------------------------------------------------------------------------
	Wnoutrefresh(pan) - debugging hook for wnoutrefresh
--------------------------------------------------------------------------*/
#ifdef TRACE
NCURSES_EXPORT(void)
_nc_Wnoutrefresh(const PANEL * pan)
{
  dPanel("wnoutrefresh", pan);
  wnoutrefresh(pan->win);
}
#endif

/*+-------------------------------------------------------------------------
	Touchpan(pan)
--------------------------------------------------------------------------*/
#ifdef TRACE
NCURSES_EXPORT(void)
_nc_Touchpan(const PANEL * pan)
{
  dPanel("Touchpan", pan);
  touchwin(pan->win);
}
#endif

/*+-------------------------------------------------------------------------
	Touchline(pan,start,count)
--------------------------------------------------------------------------*/
#ifdef TRACE
NCURSES_EXPORT(void)
_nc_Touchline(const PANEL * pan, int start, int count)
{
  char s80[80];

  sprintf(s80, "Touchline s=%d c=%d", start, count);
  dPanel(s80, pan);
  touchline(pan->win, start, count);
}
#endif

#ifndef TRACE
#  ifndef __GNUC__
     /* Some C compilers need something defined in a source file */
extern void _nc_dummy_panel(void);
void
_nc_dummy_panel(void)
{
}
#  endif
#endif