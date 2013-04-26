
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

static void
unpack_test(const char *from, const char *options, const char *se)
{
	int r;

	/* Create a work dir named after the file we're unpacking. */
	assertMakeDir(from, 0775);
	assertChdir(from);

	/*
	 * Use cpio to unpack the sample archive
	 */
	extract_reference_file(from);
	r = systemf("%s -i %s < %s >unpack.out 2>unpack.err",
	    testprog, options, from);
	failure("Error invoking %s -i %s < %s",
	    testprog, options, from);
	assertEqualInt(r, 0);

	/* Verify that nothing went to stderr. */
	if (canSymlink()) {
		failure("Error invoking %s -i %s < %s",
		    testprog, options, from);
		assertTextFileContents(se, "unpack.err");
	}

	/*
	 * Verify unpacked files.
	 */

	/* Regular file with 2 links. */
	assertIsReg("file", 0644);
	failure("%s", from);
	assertFileSize("file", 10);
	assertFileSize("linkfile", 10);
	failure("%s", from);
	assertFileNLinks("file", 2);

	/* Another name for the same file. */
	failure("%s", from);
	assertIsHardlink("linkfile", "file");
	assertFileSize("file", 10);
	assertFileSize("linkfile", 10);

	/* Symlink */
	if (canSymlink())
		assertIsSymlink("symlink", "file");

	/* dir */
	assertIsDir("dir", 0775);

	assertChdir("..");
}

DEFINE_TEST(test_gcpio_compat)
{
	assertUmask(0);

	/* Dearchive sample files with a variety of options. */
	if (canSymlink()) {
		unpack_test("test_gcpio_compat_ref.bin",
		    "--no-preserve-owner", "1 block\n");
		unpack_test("test_gcpio_compat_ref.crc",
		    "--no-preserve-owner", "2 blocks\n");
		unpack_test("test_gcpio_compat_ref.newc",
		    "--no-preserve-owner", "2 blocks\n");
		/* gcpio-2.9 only reads 6 blocks here */
		unpack_test("test_gcpio_compat_ref.ustar",
		    "--no-preserve-owner", "7 blocks\n");
	} else {
		unpack_test("test_gcpio_compat_ref_nosym.bin",
		    "--no-preserve-owner", "1 block\n");
		unpack_test("test_gcpio_compat_ref_nosym.crc",
		    "--no-preserve-owner", "2 blocks\n");
		unpack_test("test_gcpio_compat_ref_nosym.newc",
		    "--no-preserve-owner", "2 blocks\n");
		/* gcpio-2.9 only reads 6 blocks here */
		unpack_test("test_gcpio_compat_ref_nosym.ustar",
		    "--no-preserve-owner", "7 blocks\n");
	}
}