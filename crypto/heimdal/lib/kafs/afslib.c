
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

/*
 * This file is only used with AIX
 */

#include "kafs_locl.h"

int
aix_pioctl(char *a_path,
	   int o_opcode,
	   struct ViceIoctl *a_paramsP,
	   int a_followSymlinks)
{
    return lpioctl(a_path, o_opcode, a_paramsP, a_followSymlinks);
}

int
aix_setpag(void)
{
    return lsetpag();
}