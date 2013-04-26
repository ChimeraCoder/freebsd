
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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#elif defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
getdtablesize(void)
{
  int files = -1;
#if defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
  files = sysconf(_SC_OPEN_MAX);
#else /* !defined(HAVE_SYSCONF) */
#if defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
  struct rlimit res;
  if (getrlimit(RLIMIT_NOFILE, &res) == 0)
    files = res.rlim_cur;
#else /* !definded(HAVE_GETRLIMIT) */
#if defined(HAVE_SYSCTL) && defined(CTL_KERN) && defined(KERN_MAXFILES)
  int mib[2];
  size_t len;

  mib[0] = CTL_KERN;
  mib[1] = KERN_MAXFILES;
  len = sizeof(files);
  sysctl(&mib, 2, &files, sizeof(files), NULL, 0);
#endif /* defined(HAVE_SYSCTL) */
#endif /* !definded(HAVE_GETRLIMIT) */
#endif /* !defined(HAVE_SYSCONF) */

#ifdef OPEN_MAX
  if (files < 0)
    files = OPEN_MAX;
#endif

#ifdef NOFILE
  if (files < 0)
    files = NOFILE;
#endif

  return files;
}