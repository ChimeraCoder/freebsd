
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

#include <errno.h>
#include <e_os.h>

/* The routines really come from the Levitte Programming, so to make
   life simple, let's just use the raw files and hack the symbols to
   fit our namespace.  */
#define LP_DIR_CTX OPENSSL_DIR_CTX
#define LP_dir_context_st OPENSSL_dir_context_st
#define LP_find_file OPENSSL_DIR_read
#define LP_find_file_end OPENSSL_DIR_end

#include "o_dir.h"

#define LPDIR_H
#if defined OPENSSL_SYS_UNIX || defined DJGPP
#include "LPdir_unix.c"
#elif defined OPENSSL_SYS_VMS
#include "LPdir_vms.c"
#elif defined OPENSSL_SYS_WIN32
#include "LPdir_win32.c"
#elif defined OPENSSL_SYS_WINCE
#include "LPdir_wince.c"
#else
#include "LPdir_nyi.c"
#endif