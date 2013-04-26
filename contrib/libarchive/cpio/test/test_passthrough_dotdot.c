
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
 * Verify that "cpio -p .." works.
 */

DEFINE_TEST(test_passthrough_dotdot)
{
	int r;
	FILE *filelist;

	assertUmask(0);

	/*
	 * Create an assortment of files on disk.
	 */
	filelist = fopen("filelist", "w");

	/* Directory. */
	assertMakeDir("dir", 0755);
	assertChdir("dir");

	fprintf(filelist, ".\n");

	/* File with 10 bytes content. */
	assertMakeFile("file", 0642, "1234567890");
	fprintf(filelist, "file\n");

	/* All done. */
	fclose(filelist);


	/*
	 * Use cpio passthrough mode to copy files to another directory.
	 */
	r = systemf("%s -pdvm .. <../filelist >../stdout 2>../stderr",
	    testprog);
	failure("Error invoking %s -pd ..", testprog);
	assertEqualInt(r, 0);

	assertChdir("..");

	/* Verify stderr and stdout. */
	assertTextFileContents("../.\n../file\n1 block\n", "stderr");
	assertEmptyFile("stdout");

	/* Regular file. */
	assertIsReg("file", 0642);
	assertFileSize("file", 10);
	assertFileNLinks("file", 1);
}