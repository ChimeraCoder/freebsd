
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

#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: asprintf.c,v 1.4 2010/07/21 16:47:17 christos Exp $")
#endif

int asprintf(char **ptr, const char *fmt, ...)
{
  va_list vargs;
  int retval;

  va_start(vargs, fmt);
  retval = vasprintf(ptr, fmt, vargs);
  va_end(vargs);

  return retval;
}