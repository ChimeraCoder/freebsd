
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
 * This file is in public domain.
 * Written by Konstantin Belousov <kib@freebsd.org>
 *
 * $FreeBSD$
 */
#include <sys/mman.h>
#include <err.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

static void
test_pagesizes(void)
{
	size_t *ps;
	int i, nelem;

	nelem = getpagesizes(NULL, 0);
	if (nelem == -1)
		err(1, "getpagesizes(NULL, 0)");
	ps = malloc(nelem * sizeof(size_t));
	if (ps == NULL)
		err(1, "malloc");
	nelem = getpagesizes(ps, nelem);
	if (nelem == -1)
		err(1, "getpagesizes");
	printf("Supported page sizes:");
	for (i = 0; i < nelem; i++)
		printf(" %jd", (intmax_t)ps[i]);
	printf("\n");
}

static void
test_pagesize(void)
{

	printf("Pagesize: %d\n", getpagesize());
}

static void
test_osreldate(void)
{

	printf("OSRELDATE: %d\n", getosreldate());
}

static void
test_ncpus(void)
{

	printf("NCPUs: %ld\n", sysconf(_SC_NPROCESSORS_CONF));
}

int
main(int argc __unused, char *argv[] __unused)
{

	test_pagesizes();
	test_pagesize();
	test_osreldate();
	test_ncpus();
	return (0);
}