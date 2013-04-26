
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <langinfo.h>
#include <regex.h>
#include <stdlib.h>

int
rpmatch(const char *response)
{
	regex_t yes, no;
	int ret;

	if (regcomp(&yes, nl_langinfo(YESEXPR), REG_EXTENDED|REG_NOSUB) != 0)
		return (-1);
	if (regcomp(&no, nl_langinfo(NOEXPR), REG_EXTENDED|REG_NOSUB) != 0) {
		regfree(&yes);
		return (-1);
	}
	if (regexec(&yes, response, 0, NULL, 0) == 0)
		ret = 1;
	else if (regexec(&no, response, 0, NULL, 0) == 0)
		ret = 0;
	else
		ret = -1;
	regfree(&yes);
	regfree(&no);
	return (ret);
}