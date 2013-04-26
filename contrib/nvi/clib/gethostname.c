
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
 * Solaris doesn't include the gethostname call by default.
 */#include <sys/utsname.h>
#include <sys/systeminfo.h>

#include <netdb.h>

/*
 * PUBLIC: #ifndef HAVE_GETHOSTNAME
 * PUBLIC: int gethostname __P((char *, int));
 * PUBLIC: #endif
 */
int
gethostname(host, len)
	char *host;
	int len;
{
	return (sysinfo(SI_HOSTNAME, host, len) == -1 ? -1 : 0);
}