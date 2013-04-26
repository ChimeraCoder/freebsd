
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

#include <sys/types.h>

#include <login_cap.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char *
login_setcryptfmt(login_cap_t *lc, const char *def, const char *error) {
	const char *cipher;

	cipher = login_getcapstr(lc, "passwd_format", def, NULL);
	if (getenv("CRYPT_DEBUG") != NULL)
		fprintf(stderr, "login_setcryptfmt: "
		    "passwd_format = %s\n", cipher);
	if (cipher == NULL)
		return (error);
	if (!crypt_set_format(cipher))
		return (error);
	return (cipher);
}