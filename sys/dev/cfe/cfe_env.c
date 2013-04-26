
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

#include <sys/param.h>
#include <sys/kenv.h>

#include <dev/cfe/cfe_api.h>

__FBSDID("$FreeBSD$");

#ifndef	CFE_ENV_SIZE
#define	CFE_ENV_SIZE	PAGE_SIZE	/* default is one page */
#endif

extern void cfe_env_init(void);

static char cfe_env_buf[CFE_ENV_SIZE];

void
cfe_env_init(void)
{
	int idx;
	char name[KENV_MNAMELEN], val[KENV_MVALLEN];

	init_static_kenv(cfe_env_buf, CFE_ENV_SIZE);

	idx = 0;
	while (1) {
		if (cfe_enumenv(idx, name, sizeof(name), val, sizeof(val)) != 0)
			break;

		if (setenv(name, val) != 0) {
			printf("No space to store CFE env: \"%s=%s\"\n",
				name, val);
		}
		++idx;
	}
}