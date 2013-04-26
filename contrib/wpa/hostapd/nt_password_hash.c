
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

#include "includes.h"

#include "common.h"
#include "crypto/ms_funcs.h"


int main(int argc, char *argv[])
{
	unsigned char password_hash[16];
	size_t i;
	char *password, buf[64], *pos;

	if (argc > 1)
		password = argv[1];
	else {
		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			printf("Failed to read password\n");
			return 1;
		}
		buf[sizeof(buf) - 1] = '\0';
		pos = buf;
		while (*pos != '\0') {
			if (*pos == '\r' || *pos == '\n') {
				*pos = '\0';
				break;
			}
			pos++;
		}
		password = buf;
	}

	if (nt_password_hash((u8 *) password, strlen(password), password_hash))
		return -1;
	for (i = 0; i < sizeof(password_hash); i++)
		printf("%02x", password_hash[i]);
	printf("\n");

	return 0;
}