
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
* Module m_request_name                                                    *
* Routines to handle external names of menu requests                       *
***************************************************************************/

#include "menu.priv.h"

MODULE_ID("$Id: m_req_name.c,v 1.20 2008/09/13 18:59:17 tom Exp $")

static const char *request_names[MAX_MENU_COMMAND - MIN_MENU_COMMAND + 1] =
{
  "LEFT_ITEM",
  "RIGHT_ITEM",
  "UP_ITEM",
  "DOWN_ITEM",
  "SCR_ULINE",
  "SCR_DLINE",
  "SCR_DPAGE",
  "SCR_UPAGE",
  "FIRST_ITEM",
  "LAST_ITEM",
  "NEXT_ITEM",
  "PREV_ITEM",
  "TOGGLE_ITEM",
  "CLEAR_PATTERN",
  "BACK_PATTERN",
  "NEXT_MATCH",
  "PREV_MATCH"
};

#define A_SIZE (sizeof(request_names)/sizeof(request_names[0]))

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  const char * menu_request_name (int request);
|   
|   Description   :  Get the external name of a menu request.
|
|   Return Values :  Pointer to name      - on success
|                    NULL                 - on invalid request code
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(const char *)
menu_request_name(int request)
{
  T((T_CALLED("menu_request_name(%d)"), request));
  if ((request < MIN_MENU_COMMAND) || (request > MAX_MENU_COMMAND))
    {
      SET_ERROR(E_BAD_ARGUMENT);
      returnCPtr((const char *)0);
    }
  else
    returnCPtr(request_names[request - MIN_MENU_COMMAND]);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnmenu  
|   Function      :  int menu_request_by_name (const char *str);
|   
|   Description   :  Search for a request with this name.
|
|   Return Values :  Request Id       - on success
|                    E_NO_MATCH       - request not found
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
menu_request_by_name(const char *str)
{
  /* because the table is so small, it doesn't really hurt
     to run sequentially through it.
   */
  unsigned int i = 0;
  char buf[16];

  T((T_CALLED("menu_request_by_name(%s)"), _nc_visbuf(str)));

  if (str)
    {
      strncpy(buf, str, sizeof(buf));
      while ((i < sizeof(buf)) && (buf[i] != '\0'))
	{
	  buf[i] = toupper(UChar(buf[i]));
	  i++;
	}

      for (i = 0; i < A_SIZE; i++)
	{
	  if (strncmp(request_names[i], buf, sizeof(buf)) == 0)
	    returnCode(MIN_MENU_COMMAND + (int)i);
	}
    }
  RETURN(E_NO_MATCH);
}

/* m_req_name.c ends here */