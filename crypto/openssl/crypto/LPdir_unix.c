
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

#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#ifndef LPDIR_H
#include "LPdir.h"
#endif

/* The POSIXly macro for the maximum number of characters in a file path
   is NAME_MAX.  However, some operating systems use PATH_MAX instead.
   Therefore, it seems natural to first check for PATH_MAX and use that,
   and if it doesn't exist, use NAME_MAX. */
#if defined(PATH_MAX)
# define LP_ENTRY_SIZE PATH_MAX
#elif defined(NAME_MAX)
# define LP_ENTRY_SIZE NAME_MAX
#endif

/* Of course, there's the possibility that neither PATH_MAX nor NAME_MAX
   exist.  It's also possible that NAME_MAX exists but is define to a
   very small value (HP-UX offers 14), so we need to check if we got a
   result, and if it meets a minimum standard, and create or change it
   if not. */
#if !defined(LP_ENTRY_SIZE) || LP_ENTRY_SIZE<255
# undef LP_ENTRY_SIZE
# define LP_ENTRY_SIZE 255
#endif

struct LP_dir_context_st
{
  DIR *dir;
  char entry_name[LP_ENTRY_SIZE+1];
};

const char *LP_find_file(LP_DIR_CTX **ctx, const char *directory)
{
  struct dirent *direntry = NULL;

  if (ctx == NULL || directory == NULL)
    {
      errno = EINVAL;
      return 0;
    }

  errno = 0;
  if (*ctx == NULL)
    {
      *ctx = (LP_DIR_CTX *)malloc(sizeof(LP_DIR_CTX));
      if (*ctx == NULL)
	{
	  errno = ENOMEM;
	  return 0;
	}
      memset(*ctx, '\0', sizeof(LP_DIR_CTX));

      (*ctx)->dir = opendir(directory);
      if ((*ctx)->dir == NULL)
	{
	  int save_errno = errno; /* Probably not needed, but I'm paranoid */
	  free(*ctx);
	  *ctx = NULL;
	  errno = save_errno;
	  return 0;
	}
    }

  direntry = readdir((*ctx)->dir);
  if (direntry == NULL)
    {
      return 0;
    }

  strncpy((*ctx)->entry_name, direntry->d_name, sizeof((*ctx)->entry_name) - 1);
  (*ctx)->entry_name[sizeof((*ctx)->entry_name) - 1] = '\0';
  return (*ctx)->entry_name;
}

int LP_find_file_end(LP_DIR_CTX **ctx)
{
  if (ctx != NULL && *ctx != NULL)
    {
      int ret = closedir((*ctx)->dir);

      free(*ctx);
      switch (ret)
	{
	case 0:
	  return 1;
	case -1:
	  return 0;
	default:
	  break;
	}
    }
  errno = EINVAL;
  return 0;
}