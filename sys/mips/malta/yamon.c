
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

#include <sys/param.h>
#include <sys/systm.h>

#include <mips/malta/yamon.h>

char *
yamon_getenv(char *name)
{
	char *value;
	yamon_env_t *p;

	value = NULL;
	for (p = *fenvp; p->name != NULL; ++p) {
	    if (!strcmp(name, p->name)) {
		value = p->value;
		break;
	    }
	}

	return (value);
}

uint32_t
yamon_getcpufreq(void)
{
	uint32_t freq;
	int ret;

	ret = YAMON_SYSCON_READ(SYSCON_BOARD_CPU_CLOCK_FREQ_ID, &freq,
	    sizeof(freq));
	if (ret != 0)
		freq = 0;

	return (freq);
}