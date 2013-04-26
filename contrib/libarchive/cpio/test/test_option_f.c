
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

/*
 * Unpack the archive in a new dir.
 */
static void
unpack(const char *dirname, const char *option)
{
	int r;

	assertMakeDir(dirname, 0755);
	assertChdir(dirname);
	extract_reference_file("test_option_f.cpio");
	r = systemf("%s -i %s < test_option_f.cpio > copy-no-a.out 2>copy-no-a.err", testprog, option);
	assertEqualInt(0, r);
	assertChdir("..");
}

DEFINE_TEST(test_option_f)
{
	/* Calibrate:  No -f option, so everything should be extracted. */
	unpack("t0", "--no-preserve-owner");
	assertFileExists("t0/a123");
	assertFileExists("t0/a234");
	assertFileExists("t0/b123");
	assertFileExists("t0/b234");

	/* Don't extract 'a*' files. */
#if defined(_WIN32) && !defined(__CYGWIN__)
	/* Single quotes isn't used by command.exe. */
	unpack("t1", "--no-preserve-owner -f a*");
#else
	unpack("t1", "--no-preserve-owner -f 'a*'");
#endif
	assertFileNotExists("t1/a123");
	assertFileNotExists("t1/a234");
	assertFileExists("t1/b123");
	assertFileExists("t1/b234");

	/* Don't extract 'b*' files. */
#if defined(_WIN32) && !defined(__CYGWIN__)
	/* Single quotes isn't used by command.exe. */
	unpack("t2", "--no-preserve-owner -f b*");
#else
	unpack("t2", "--no-preserve-owner -f 'b*'");
#endif
	assertFileExists("t2/a123");
	assertFileExists("t2/a234");
	assertFileNotExists("t2/b123");
	assertFileNotExists("t2/b234");
}