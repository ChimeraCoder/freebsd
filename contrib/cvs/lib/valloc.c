
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
/* valloc -- return memory aligned to the page size.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "system.h"

#ifndef HAVE_GETPAGESIZE
# include "getpagesize.h"
#endif

void *
valloc (bytes)
     size_t bytes;
{
  long pagesize;
  char *ret;

  pagesize = getpagesize ();
  ret = (char *) malloc (bytes + pagesize - 1);
  if (ret)
    ret = (char *) ((long) (ret + pagesize - 1) &~ (pagesize - 1));
  return ret;
}