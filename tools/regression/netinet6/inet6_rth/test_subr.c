
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_subr.h"

int g_total;
int g_pass;
int g_fail;
char g_funcname[FUNCNAMESIZE];
char g_testdesc[LINESIZE];
char g_errbuf[LINESIZE];

void
set_funcname(char *bufp, size_t bufsize)
{
	strlcpy(g_funcname, bufp,
	    bufsize < FUNCNAMESIZE ? bufsize : FUNCNAMESIZE);
}

/*
 * desc is a NULL-terminated string.
 */
void
checkptr(caddr_t expected, caddr_t got, const char *desc)
{
	int  len;
	int  failed;
	char sbuf[LINESIZE];

	memset((void *)sbuf, 0, LINESIZE);
	snprintf(g_testdesc, LINESIZE, desc);

	failed = 1;
	g_total++;
	if (got == expected) {
		len = snprintf(sbuf, LINESIZE, "ok");
		g_pass++;
		failed = 0;
	} else {
		len = snprintf(sbuf, LINESIZE, "not ok");
		snprintf(g_errbuf, LINESIZE, " : Expected %#x, but got %#x",
		    (unsigned int)expected, (unsigned int)got);
		g_fail++;
	}
	snprintf(sbuf + len, LINESIZE - len, " %d - %s (%s)",
	    g_total, g_funcname, g_testdesc);
	printf(sbuf);
	if (failed)
		printf(g_errbuf);
	printf("\n");
	fflush(NULL);
	memset((void *)g_errbuf, 0, LINESIZE);
	memset((void *)g_testdesc, 0, LINESIZE);
}

void
checkstr(const char *expected, const char *got, size_t explen, const char *desc)
{
	int  len;
	int  failed;
	char sbuf[LINESIZE];

	memset((void *)sbuf, 0, LINESIZE);
	snprintf(g_testdesc, LINESIZE, desc);

	failed = 1;
	g_total++;
	if (strncmp(expected, got, explen) == 0) {
		len = snprintf(sbuf, LINESIZE, "ok");
		g_pass++;
		failed = 0;
	} else {
		len = snprintf(sbuf, LINESIZE, "not ok");
		snprintf(g_errbuf, LINESIZE,
		    " : Expected %s, but got %s", expected, got);
		g_fail++;
	}
	snprintf(sbuf + len, LINESIZE - len, " %d - %s (%s)",
	    g_total, g_funcname, g_testdesc);
	printf(sbuf);
	if (failed)
		printf(g_errbuf);
	printf("\n");
	fflush(NULL);
	memset((void *)g_errbuf, 0, LINESIZE);
	memset((void *)g_testdesc, 0, LINESIZE);
}

void
checknum(int expected, int got, int cmp, const char *desc)
{
	int  len;
	int  pass;
	int  failed;
	char sbuf[LINESIZE];

	memset((void *)sbuf, 0, LINESIZE);
	snprintf(g_testdesc, LINESIZE, desc);

	failed = 1;
	pass = 0;
	g_total++;
	switch(cmp) {
	case 0:
		pass = (got == expected) ? 1 : 0;
		break;
	case 1:
		pass = (got > expected) ? 1 : 0;
		break;
	case -1:
		pass = (got < expected) ? 1 : 0;
		break;
	}
	if (pass != 0) {
		len = snprintf(sbuf, LINESIZE, "ok");
		g_pass++;
		failed = 0;
	} else {
		len = snprintf(sbuf, LINESIZE, "not ok");
		snprintf(g_errbuf, LINESIZE,
		    " : Expected %d, but got %d", expected, got);
		g_fail++;
	}
	snprintf(sbuf + len, LINESIZE - len, " %d - %s (%s)",
	    g_total, g_funcname, g_testdesc);
	printf(sbuf);
	if (failed)
		printf(g_errbuf);
	printf("\n");
	fflush(NULL);
	memset((void *)g_errbuf, 0, LINESIZE);
	memset((void *)g_testdesc, 0, LINESIZE);
}