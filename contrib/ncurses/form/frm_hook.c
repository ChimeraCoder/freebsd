
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

MODULE_ID("$Id: frm_hook.c,v 1.14 2004/12/25 22:37:27 tom Exp $")

/* "Template" macro to generate function to set application specific hook */
#define GEN_HOOK_SET_FUNCTION( typ, name ) \
NCURSES_IMPEXP int NCURSES_API set_ ## typ ## _ ## name (FORM *form, Form_Hook func)\
{\
   T((T_CALLED("set_" #typ"_"#name"(%p,%p)"), form, func));\
   (Normalize_Form( form ) -> typ ## name) = func ;\
   RETURN(E_OK);\
}

/* "Template" macro to generate function to get application specific hook */
#define GEN_HOOK_GET_FUNCTION( typ, name ) \
NCURSES_IMPEXP Form_Hook NCURSES_API typ ## _ ## name ( const FORM *form )\
{\
   T((T_CALLED(#typ "_" #name "(%p)"), form));\
   returnFormHook( Normalize_Form( form ) -> typ ## name );\
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int set_field_init(FORM *form, Form_Hook f)
|
|   Description   :  Assigns an application defined initialization function
|                    to be called when the form is posted and just after
|                    the current field changes.
|
|   Return Values :  E_OK      - success
+--------------------------------------------------------------------------*/
GEN_HOOK_SET_FUNCTION(field, init)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  Form_Hook field_init(const FORM *form)
|
|   Description   :  Retrieve field initialization routine address.
|
|   Return Values :  The address or NULL if no hook defined.
+--------------------------------------------------------------------------*/
GEN_HOOK_GET_FUNCTION(field, init)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int set_field_term(FORM *form, Form_Hook f)
|
|   Description   :  Assigns an application defined finalization function
|                    to be called when the form is unposted and just before
|                    the current field changes.
|
|   Return Values :  E_OK      - success
+--------------------------------------------------------------------------*/
GEN_HOOK_SET_FUNCTION(field, term)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  Form_Hook field_term(const FORM *form)
|
|   Description   :  Retrieve field finalization routine address.
|
|   Return Values :  The address or NULL if no hook defined.
+--------------------------------------------------------------------------*/
GEN_HOOK_GET_FUNCTION(field, term)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int set_form_init(FORM *form, Form_Hook f)
|
|   Description   :  Assigns an application defined initialization function
|                    to be called when the form is posted and just after
|                    a page change.
|
|   Return Values :  E_OK       - success
+--------------------------------------------------------------------------*/
GEN_HOOK_SET_FUNCTION(form, init)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  Form_Hook form_init(const FORM *form)
|
|   Description   :  Retrieve form initialization routine address.
|
|   Return Values :  The address or NULL if no hook defined.
+--------------------------------------------------------------------------*/
GEN_HOOK_GET_FUNCTION(form, init)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  int set_form_term(FORM *form, Form_Hook f)
|
|   Description   :  Assigns an application defined finalization function
|                    to be called when the form is unposted and just before
|                    a page change.
|
|   Return Values :  E_OK       - success
+--------------------------------------------------------------------------*/
GEN_HOOK_SET_FUNCTION(form, term)

/*---------------------------------------------------------------------------
|   Facility      :  libnform
|   Function      :  Form_Hook form_term(const FORM *form)
|
|   Description   :  Retrieve form finalization routine address.
|
|   Return Values :  The address or NULL if no hook defined.
+--------------------------------------------------------------------------*/
GEN_HOOK_GET_FUNCTION(form, term)

/* frm_hook.c ends here */