
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

/* This is a little pointless, as Windows doesn't support symlinks
 * (except for the seriously crippled CreateSymbolicLink API) so these
 * tests won't run on Windows. */
#if defined(_WIN32) && !defined(__CYGWIN__)
#define CAT "type"
#else
#define CAT "cat"
#endif

DEFINE_TEST(test_option_L_upper)
{
	FILE *filelist;
	int r;

	if (!canSymlink()) {
		skipping("Symlink tests");
		return;
	}

	filelist = fopen("filelist", "w");

	/* Create a file and a symlink to the file. */
	assertMakeFile("file", 0644, "1234567890");
	fprintf(filelist, "file\n");

	/* Symlink to above file. */
	assertMakeSymlink("symlink", "file");
	fprintf(filelist, "symlink\n");

	fclose(filelist);

	r = systemf(CAT " filelist | %s -pd copy >copy.out 2>copy.err", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "copy.err");

	failure("Regular -p without -L should preserve symlinks.");
	assertIsSymlink("copy/symlink", NULL);

	r = systemf(CAT " filelist | %s -pd -L copy-L >copy-L.out 2>copy-L.err", testprog);
	assertEqualInt(r, 0);
	assertEmptyFile("copy-L.out");
	assertTextFileContents("1 block\n", "copy-L.err");
	failure("-pdL should dereference symlinks and turn them into files.");
	assertIsReg("copy-L/symlink", -1);

	r = systemf(CAT " filelist | %s -o >archive.out 2>archive.err", testprog);
	failure("Error invoking %s -o ", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "archive.err");

	assertMakeDir("unpack", 0755);
	assertChdir("unpack");
	r = systemf(CAT " ../archive.out | %s -i >unpack.out 2>unpack.err", testprog);
	failure("Error invoking %s -i", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "unpack.err");
	assertChdir("..");

	assertIsSymlink("unpack/symlink", NULL);

	r = systemf(CAT " filelist | %s -oL >archive-L.out 2>archive-L.err", testprog);
	failure("Error invoking %s -oL", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "archive-L.err");

	assertMakeDir("unpack-L", 0755);
	assertChdir("unpack-L");
	r = systemf(CAT " ../archive-L.out | %s -i >unpack-L.out 2>unpack-L.err", testprog);
	failure("Error invoking %s -i < archive-L.out", testprog);
	assertEqualInt(r, 0);
	assertTextFileContents("1 block\n", "unpack-L.err");
	assertChdir("..");
	assertIsReg("unpack-L/symlink", -1);
}