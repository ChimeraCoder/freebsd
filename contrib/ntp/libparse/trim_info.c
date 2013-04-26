
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
#include "ntp_types.h"
#include "trimble.h"

cmd_info_t *
trimble_convert(
		unsigned int cmd,
		cmd_info_t   *tbl
		)
{
  int i;

  for (i = 0; tbl[i].cmd != 0xFF; i++)
    {
      if (tbl[i].cmd == cmd)
	return &tbl[i];
    }
  return 0;
}

/*
 * trim_info.c,v
 * Revision 4.5  2005/04/16 17:32:10  kardel
 * update copyright
 *
 * Revision 4.4  2004/11/14 15:29:41  kardel
 * support PPSAPI, upgrade Copyright to Berkeley style
 *
 * Revision 4.2  1998/12/20 23:45:31  kardel
 * fix types and warnings
 *
 * Revision 4.1  1998/08/09 22:27:48  kardel
 * Trimble TSIP support
 *
 */