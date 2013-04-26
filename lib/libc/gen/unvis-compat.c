
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

#include <vis.h>

#define	_UNVIS_END	1

int
__unvis_44bsd(char *cp, int c, int *astate, int flag)
{

	if (flag & _UNVIS_END)
		flag = (flag & ~_UNVIS_END) ^ UNVIS_END;
	return unvis(cp, c, astate, flag);
}

__sym_compat(unvis, __vis_44bsd, FBSD_1.0);