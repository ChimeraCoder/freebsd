
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

/***************************************************************************
*                                                                          *
*  Author : Juergen Pfeifer                                                *
*                                                                          *
***************************************************************************/

#include "form.priv.h"

MODULE_ID("$Id: fty_alpha.c,v 1.23 2007/10/13 19:32:09 tom Exp $")

#define thisARG alphaARG

typedef struct
  {
    int width;
  }
thisARG;

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  static void *Make_This_Type(va_list *ap)
|
|   Description   :  Allocate structure for alpha type argument.
|
|   Return Values :  Pointer to argument structure or NULL on error
+--------------------------------------------------------------------------*/
static void *
Make_This_Type(va_list *ap)
{
  thisARG *argp = typeMalloc(thisARG, 1);

  if (argp)
    {
      T((T_CREATE("thisARG %p"), argp));
      argp->width = va_arg(*ap, int);
    }

  return ((void *)argp);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  static void *Copy_This_Type(const void * argp)
|
|   Description   :  Copy structure for alpha type argument.
|
|   Return Values :  Pointer to argument structure or NULL on error.
+--------------------------------------------------------------------------*/
static void *
Copy_This_Type(const void *argp)
{
  const thisARG *ap = (const thisARG *)argp;
  thisARG *result = typeMalloc(thisARG, 1);

  if (result)
    {
      T((T_CREATE("thisARG %p"), result));
      *result = *ap;
    }

  return ((void *)result);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  static void Free_This_Type(void *argp)
|
|   Description   :  Free structure for alpha type argument.
|
|   Return Values :  -
+--------------------------------------------------------------------------*/
static void
Free_This_Type(void *argp)
{
  if (argp)
    free(argp);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  static bool Check_This_Character(
|                                      int c,
|                                      const void *argp)
|
|   Description   :  Check a character for the alpha type.
|
|   Return Values :  TRUE  - character is valid
|                    FALSE - character is invalid
+--------------------------------------------------------------------------*/
static bool
Check_This_Character(int c, const void *argp GCC_UNUSED)
{
#if USE_WIDEC_SUPPORT
  if (iswalpha((wint_t) c))
    return TRUE;
#endif
  return (isalpha(UChar(c)) ? TRUE : FALSE);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  static bool Check_This_Field(
|                                      FIELD *field,
|                                      const void *argp)
|
|   Description   :  Validate buffer content to be a valid alpha value
|
|   Return Values :  TRUE  - field is valid
|                    FALSE - field is invalid
+--------------------------------------------------------------------------*/
static bool
Check_This_Field(FIELD *field, const void *argp)
{
  int width = ((const thisARG *)argp)->width;
  unsigned char *bp = (unsigned char *)field_buffer(field, 0);
  bool result = (width < 0);

  Check_CTYPE_Field(result, bp, width, Check_This_Character);
  return (result);
}

static FIELDTYPE typeTHIS =
{
  _HAS_ARGS | _RESIDENT,
  1,				/* this is mutable, so we can't be const */
  (FIELDTYPE *)0,
  (FIELDTYPE *)0,
  Make_This_Type,
  Copy_This_Type,
  Free_This_Type,
  Check_This_Field,
  Check_This_Character,
  NULL,
  NULL
};

NCURSES_EXPORT_VAR(FIELDTYPE*) TYPE_ALPHA = &typeTHIS;

/* fty_alpha.c ends here */