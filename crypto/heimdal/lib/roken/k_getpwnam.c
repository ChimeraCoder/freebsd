
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

#include <config.h>

#include "roken.h"
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

ROKEN_LIB_FUNCTION struct passwd * ROKEN_LIB_CALL
k_getpwnam (const char *user)
{
     struct passwd *p;

     p = getpwnam (user);
#if defined(HAVE_GETSPNAM) && defined(HAVE_STRUCT_SPWD)
     if(p)
     {
	  struct spwd *spwd;

	  spwd = getspnam (user);
	  if (spwd)
	       p->pw_passwd = spwd->sp_pwdp;
	  endspent ();
     }
#else
     endpwent ();
#endif
     return p;
}