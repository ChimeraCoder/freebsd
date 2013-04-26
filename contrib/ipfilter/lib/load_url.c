
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

#include "ipf.h"

alist_t *
load_url(char *url)
{
	alist_t *hosts = NULL;

	if (strncmp(url, "file://", 7) == 0) { 
		/*      
		 * file:///etc/passwd
		 *        ^------------s
		 */     
		hosts = load_file(url);

	} else if (*url == '/' || *url == '.') {
		hosts = load_file(url);

	} else if (strncmp(url, "http://", 7) == 0) {
		hosts = load_http(url);
	}

	return hosts;  
}