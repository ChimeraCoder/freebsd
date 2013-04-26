
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
#include <string.h>
#include <libutil.h>
#include <unistd.h>
#include "crypt.h"

static const struct {
	const char *const name;
	char *(*const func)(const char *, const char *);
	const char *const magic;
} crypt_types[] = {
#ifdef HAS_DES
	{
		"des",
		crypt_des,
		NULL
	},
#endif
	{
		"md5",
		crypt_md5,
		"$1$"
	},
#ifdef HAS_BLOWFISH
	{
		"blf",
		crypt_blowfish,
		"$2"
	},
#endif
	{
		"nth",
		crypt_nthash,
		"$3$"
	},
	{
		"sha256",
		crypt_sha256,
		"$5$"
	},
	{
		"sha512",
		crypt_sha512,
		"$6$"
	},
	{
		NULL,
		NULL,
		NULL
	}
};

#ifdef HAS_DES
#define CRYPT_DEFAULT	"des"
#else
#define CRYPT_DEFAULT	"md5"
#endif

static int crypt_type = -1;

static void
crypt_setdefault(void)
{
	size_t i;

	if (crypt_type != -1)
		return;
	for (i = 0; i < sizeof(crypt_types) / sizeof(crypt_types[0]) - 1; i++) {
		if (strcmp(CRYPT_DEFAULT, crypt_types[i].name) == 0) {
			crypt_type = (int)i;
			return;
		}
	}
	crypt_type = 0;
}

const char *
crypt_get_format(void)
{

	crypt_setdefault();
	return (crypt_types[crypt_type].name);
}

int
crypt_set_format(const char *type)
{
	size_t i;

	crypt_setdefault();
	for (i = 0; i < sizeof(crypt_types) / sizeof(crypt_types[0]) - 1; i++) {
		if (strcmp(type, crypt_types[i].name) == 0) {
			crypt_type = (int)i;
			return (1);
		}
	}
	return (0);
}

char *
crypt(const char *passwd, const char *salt)
{
	size_t i;

	crypt_setdefault();
	for (i = 0; i < sizeof(crypt_types) / sizeof(crypt_types[0]) - 1; i++) {
		if (crypt_types[i].magic != NULL && strncmp(salt,
		    crypt_types[i].magic, strlen(crypt_types[i].magic)) == 0)
			return (crypt_types[i].func(passwd, salt));
	}
	return (crypt_types[crypt_type].func(passwd, salt));
}