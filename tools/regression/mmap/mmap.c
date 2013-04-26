
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
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include <stdio.h>
#include <err.h>

const struct tests {
	void	*addr;
	int	ok[2];	/* Depending on security.bsd.map_at_zero {0, !=0}. */
} tests[] = {
	{ (void *)0,			{ 0, 1 } }, /* Test sysctl. */
	{ (void *)1,			{ 0, 0 } },
	{ (void *)(PAGE_SIZE - 1),	{ 0, 0 } },
	{ (void *)PAGE_SIZE,		{ 1, 1 } },
	{ (void *)-1,			{ 0, 0 } },
	{ (void *)(-PAGE_SIZE),		{ 0, 0 } },
	{ (void *)(-1 - PAGE_SIZE),	{ 0, 0 } },
	{ (void *)(-1 - PAGE_SIZE - 1),	{ 0, 0 } },
	{ (void *)(0x1000 * PAGE_SIZE),	{ 1, 1 } },
};

int
main(void)
{
	void *p;
	size_t len;
	int i, error, mib[3], map_at_zero;

	error = 0;

	/* Get the current sysctl value of security.bsd.map_at_zero. */
	len = sizeof(mib) / sizeof(*mib);
	if (sysctlnametomib("security.bsd.map_at_zero", mib, &len) == -1)
		err(1, "sysctlnametomib(security.bsd.map_at_zero)");

	len = sizeof(map_at_zero);
	if (sysctl(mib, 3, &map_at_zero, &len, NULL, 0) == -1)
		err(1, "sysctl(security.bsd.map_at_zero)");

	/* Normalize to 0 or 1 for array access. */
	map_at_zero = !!map_at_zero;

	for (i=0; i < (sizeof(tests) / sizeof(*tests)); i++) {
		p = mmap((void *)tests[i].addr, PAGE_SIZE,
		    PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_FIXED,
		    -1, 0);
		if (p == MAP_FAILED) {
			if (tests[i].ok[map_at_zero] != 0)
				error++;
			warnx("%s: mmap(%p, ...) failed.",
			    (tests[i].ok[map_at_zero] == 0) ? "OK " : "ERR",
			     tests[i].addr);
		} else {
			if (tests[i].ok[map_at_zero] != 1)
				error++;
			warnx("%s: mmap(%p, ...) succeeded: p=%p",
			    (tests[i].ok[map_at_zero] == 1) ? "OK " : "ERR",
			    tests[i].addr, p);
		}
	}

	if (error)
		err(1, "---\nERROR: %d unexpected results.", error);

	return (error != 0);
}