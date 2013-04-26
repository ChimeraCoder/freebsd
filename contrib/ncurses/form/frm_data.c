
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

MODULE_ID("$Id: frm_data.c,v 1.14 2005/11/26 15:34:01 tom Exp $")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  bool data_behind(const FORM *form)
|   
|   Description   :  Check for off-screen data behind. This is nearly trivial
|                    because the beginning of a field is fixed.
|
|   Return Values :  TRUE   - there are off-screen data behind
|                    FALSE  - there are no off-screen data behind
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(bool)
data_behind(const FORM *form)
{
  bool result = FALSE;

  T((T_CALLED("data_behind(%p)"), form));

  if (form && (form->status & _POSTED) && form->current)
    {
      FIELD *field;

      field = form->current;
      if (!Single_Line_Field(field))
	{
	  result = (form->toprow == 0) ? FALSE : TRUE;
	}
      else
	{
	  result = (form->begincol == 0) ? FALSE : TRUE;
	}
    }
  returnBool(result);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static char * Only_Padding(
|                                    WINDOW *w,
|                                    int len,
|                                    int pad)
|   
|   Description   :  Test if 'length' cells starting at the current position
|                    contain a padding character.
|
|   Return Values :  true if only padding cells are found
+--------------------------------------------------------------------------*/
NCURSES_INLINE static bool
Only_Padding(WINDOW *w, int len, int pad)
{
  bool result = TRUE;
  int y, x, j;
  FIELD_CELL cell;

  getyx(w, y, x);
  for (j = 0; j < len; ++j)
    {
      if (wmove(w, y, x + j) != ERR)
	{
#if USE_WIDEC_SUPPORT
	  if (win_wch(w, &cell) != ERR)
	    {
	      if ((chtype)CharOf(cell) != ChCharOf(pad)
		  || cell.chars[1] != 0)
		{
		  result = FALSE;
		  break;
		}
	    }
#else
	  cell = winch(w);
	  if (ChCharOf(cell) != ChCharOf(pad))
	    {
	      result = FALSE;
	      break;
	    }
#endif
	}
      else
	{
	  /* if an error, return true: no non-padding text found */
	  break;
	}
    }
  /* no need to reset the cursor position; caller does this */
  return result;
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  bool data_ahead(const FORM *form)
|   
|   Description   :  Check for off-screen data ahead. This is more difficult
|                    because a dynamic field has a variable end. 
|
|   Return Values :  TRUE   - there are off-screen data ahead
|                    FALSE  - there are no off-screen data ahead
+--------------------------------------------------------------------------*/
NCURSES_EXPORT(bool)
data_ahead(const FORM *form)
{
  bool result = FALSE;

  T((T_CALLED("data_ahead(%p)"), form));

  if (form && (form->status & _POSTED) && form->current)
    {
      FIELD *field;
      bool cursor_moved = FALSE;
      int pos;

      field = form->current;
      assert(form->w);

      if (Single_Line_Field(field))
	{
	  int check_len;

	  pos = form->begincol + field->cols;
	  while (pos < field->dcols)
	    {
	      check_len = field->dcols - pos;
	      if (check_len >= field->cols)
		check_len = field->cols;
	      cursor_moved = TRUE;
	      wmove(form->w, 0, pos);
	      if (Only_Padding(form->w, check_len, field->pad))
		pos += field->cols;
	      else
		{
		  result = TRUE;
		  break;
		}
	    }
	}
      else
	{
	  pos = form->toprow + field->rows;
	  while (pos < field->drows)
	    {
	      cursor_moved = TRUE;
	      wmove(form->w, pos, 0);
	      pos++;
	      if (!Only_Padding(form->w, field->cols, field->pad))
		{
		  result = TRUE;
		  break;
		}
	    }
	}

      if (cursor_moved)
	wmove(form->w, form->currow, form->curcol);
    }
  returnBool(result);
}

/* frm_data.c ends here */