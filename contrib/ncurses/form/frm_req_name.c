
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
* Module form_request_name                                                 *
* Routines to handle external names of menu requests                       *
***************************************************************************/

#include "form.priv.h"

MODULE_ID("$Id: frm_req_name.c,v 1.16 2008/07/05 23:22:08 tom Exp $")

static const char *request_names[MAX_FORM_COMMAND - MIN_FORM_COMMAND + 1] =
{
  "NEXT_PAGE",
  "PREV_PAGE",
  "FIRST_PAGE",
  "LAST_PAGE",

  "NEXT_FIELD",
  "PREV_FIELD",
  "FIRST_FIELD",
  "LAST_FIELD",
  "SNEXT_FIELD",
  "SPREV_FIELD",
  "SFIRST_FIELD",
  "SLAST_FIELD",
  "LEFT_FIELD",
  "RIGHT_FIELD",
  "UP_FIELD",
  "DOWN_FIELD",

  "NEXT_CHAR",
  "PREV_CHAR",
  "NEXT_LINE",
  "PREV_LINE",
  "NEXT_WORD",
  "PREV_WORD",
  "BEG_FIELD",
  "END_FIELD",
  "BEG_LINE",
  "END_LINE",
  "LEFT_CHAR",
  "RIGHT_CHAR",
  "UP_CHAR",
  "DOWN_CHAR",

  "NEW_LINE",
  "INS_CHAR",
  "INS_LINE",
  "DEL_CHAR",
  "DEL_PREV",
  "DEL_LINE",
  "DEL_WORD",
  "CLR_EOL",
  "CLR_EOF",
  "CLR_FIELD",
  "OVL_MODE",
  "INS_MODE",
  "SCR_FLINE",
  "SCR_BLINE",
  "SCR_FPAGE",
  "SCR_BPAGE",
  "SCR_FHPAGE",
  "SCR_BHPAGE",
  "SCR_FCHAR",
  "SCR_BCHAR",
  "SCR_HFLINE",
  "SCR_HBLINE",
  "SCR_HFHALF",
  "SCR_HBHALF",

  "VALIDATION",
  "NEXT_CHOICE",
  "PREV_CHOICE"
};

#define A_SIZE (sizeof(request_names)/sizeof(request_names[0]))

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  const char * form_request_name (int request);
|   
|   Description   :  Get the external name of a form request.
|
|   Return Values :  Pointer to name      - on success
|                    NULL                 - on invalid request code
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(const char *)
form_request_name(int request)
{
  T((T_CALLED("form_request_name(%d)"), request));

  if ((request < MIN_FORM_COMMAND) || (request > MAX_FORM_COMMAND))
    {
      SET_ERROR(E_BAD_ARGUMENT);
      returnCPtr((const char *)0);
    }
  else
    returnCPtr(request_names[request - MIN_FORM_COMMAND]);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int form_request_by_name (const char *str);
|   
|   Description   :  Search for a request with this name.
|
|   Return Values :  Request Id       - on success
|                    E_NO_MATCH       - request not found
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(int)
form_request_by_name(const char *str)
{
  /* because the table is so small, it doesn't really hurt
     to run sequentially through it.
   */
  unsigned int i = 0;
  char buf[16];

  T((T_CALLED("form_request_by_name(%s)"), _nc_visbuf(str)));

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
	    returnCode(MIN_FORM_COMMAND + (int) i);
	}
    }
  RETURN(E_NO_MATCH);
}

/* frm_req_name.c ends here */