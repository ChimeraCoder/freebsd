
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

#include "ftp_locl.h"
RCSID("$Id$");

#if defined(KRB5)

void
afslog(int argc, char **argv)
{
    int ret;
    if(argc > 2) {
	printf("usage: %s [cell]\n", argv[0]);
	code = -1;
	return;
    }
    if(argc == 2)
	ret = command("SITE AFSLOG %s", argv[1]);
    else
	ret = command("SITE AFSLOG");
    code = (ret == COMPLETE);
}

#else
int ftp_afslog_placeholder;
#endif