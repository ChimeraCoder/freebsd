
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amu.h>


/*
 * Convert errno to a string
 */
char *
strerror(int errnum)
{
#ifdef HAVE_EXTERN_SYS_ERRLIST
  if (errnum < 0 || errnum >= (sizeof(sys_errlist) >> 2)) {
    static char errstr[30];
    xsnprintf(errstr, sizeof(errstr), "Unknown error #%d", errnum);
    return errstr;
  }
  return sys_errlist[error];
#else  /* not HAVE_EXTERN_SYS_ERRLIST */
  return "unknown (strerror not available)";
#endif /* not HAVE_EXTERN_SYS_ERRLIST */
}