
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
#include "test.h"
__FBSDID("$FreeBSD$");

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

DEFINE_TEST(test_option_t)
{
	char *p;
	int r;
	time_t mtime;
	char date[32];
	char date2[32];

	/* List reference archive, make sure the TOC is correct. */
	extract_reference_file("test_option_t.cpio");
	r = systemf("%s -it < test_option_t.cpio >it.out 2>it.err", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "it.err");
	extract_reference_file("test_option_t.stdout");
	p = slurpfile(NULL, "test_option_t.stdout");
	assertTextFileContents(p, "it.out");
	free(p);

	/* We accept plain "-t" as a synonym for "-it" */
	r = systemf("%s -t < test_option_t.cpio >t.out 2>t.err", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "t.err");
	extract_reference_file("test_option_t.stdout");
	p = slurpfile(NULL, "test_option_t.stdout");
	assertTextFileContents(p, "t.out");
	free(p);

	/* But "-ot" is an error. */
	assert(0 != systemf("%s -ot < test_option_t.cpio >ot.out 2>ot.err",
			    testprog));
	assertEmptyFile("ot.out");

	/* List reference archive verbosely, make sure the TOC is correct. */
	r = systemf("%s -itv < test_option_t.cpio >tv.out 2>tv.err", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "tv.err");
	extract_reference_file("test_option_tv.stdout");

	/* This doesn't work because the usernames on different systems
	 * are different and cpio now looks up numeric UIDs on
	 * the local system. */
	/* assertEqualFile("tv.out", "test_option_tv.stdout"); */

	/* List reference archive with numeric IDs, verify TOC is correct. */
	r = systemf("%s -itnv < test_option_t.cpio >itnv.out 2>itnv.err",
		    testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "itnv.err");
	p = slurpfile(NULL, "itnv.out");
	/* Since -n uses numeric UID/GID, this part should be the
	 * same on every system. */
	assertEqualMem(p, "-rw-r--r--   1 1000     1000            0 ",42);

	/* Date varies depending on local timezone and locale. */
	mtime = 1;
#ifdef HAVE_LOCALE_H
	setlocale(LC_ALL, "");
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	strftime(date2, sizeof(date), "%b %d  %Y", localtime(&mtime));
	_snprintf(date, sizeof(date)-1, "%12s file", date2);
#else
	strftime(date2, sizeof(date), "%b %e  %Y", localtime(&mtime));
	snprintf(date, sizeof(date)-1, "%12s file", date2);
#endif
	assertEqualMem(p + 42, date, strlen(date));
	free(p);

	/* But "-n" without "-t" is an error. */
	assert(0 != systemf("%s -in < test_option_t.cpio >in.out 2>in.err",
			    testprog));
	assertEmptyFile("in.out");
}