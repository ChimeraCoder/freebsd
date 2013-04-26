
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
 * As reported by Bernd Walter:  Some people are in the habit of
 * using "find -d" to generate a list for cpio -p because that
 * copies the top-level dir last, which preserves owner and mode
 * information.  That's not necessary for bsdcpio (libarchive defers
 * restoring directory information), but bsdcpio should still generate
 * the correct results with this usage.
 */

DEFINE_TEST(test_passthrough_reverse)
{
	int r;
	FILE *filelist;

	assertUmask(0);

	/*
	 * Create an assortment of files on disk.
	 */
	filelist = fopen("filelist", "w");

	/* Directory. */
	assertMakeDir("dir", 0743);

	/* File with 10 bytes content. */
	assertMakeFile("dir/file", 0644, "1234567890");
	fprintf(filelist, "dir/file\n");

	/* Write dir last. */
	fprintf(filelist, "dir\n");

	/* All done. */
	fclose(filelist);


	/*
	 * Use cpio passthrough mode to copy files to another directory.
	 */
	r = systemf("%s -pdvm out <filelist >stdout 2>stderr", testprog);
	failure("Error invoking %s -pd out", testprog);
	assertEqualInt(r, 0);

	assertChdir("out");

	/* Verify stderr and stdout. */
	assertTextFileContents("out/dir/file\nout/dir\n1 block\n",
	    "../stderr");
	assertEmptyFile("../stdout");

	/* dir */
	assertIsDir("dir", 0743);


	/* Regular file. */
	assertIsReg("dir/file", 0644);
	assertFileSize("dir/file", 10);
	assertFileNLinks("dir/file", 1);
}