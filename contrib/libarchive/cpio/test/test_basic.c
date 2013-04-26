
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
verify_files(const char *msg)
{
	/*
	 * Verify unpacked files.
	 */

	/* Regular file with 2 links. */
	failure(msg);
	assertIsReg("file", 0644);
	failure(msg);
	assertFileSize("file", 10);
	failure(msg);
	assertFileNLinks("file", 2);

	/* Another name for the same file. */
	failure(msg);
	assertIsHardlink("linkfile", "file");

	/* Symlink */
	if (canSymlink())
		assertIsSymlink("symlink", "file");

	/* Another file with 1 link and different permissions. */
	failure(msg);
	assertIsReg("file2", 0777);
	failure(msg);
	assertFileSize("file2", 10);
	failure(msg);
	assertFileNLinks("file2", 1);

	/* dir */
	assertIsDir("dir", 0775);
}

static void
basic_cpio(const char *target,
    const char *pack_options,
    const char *unpack_options,
    const char *se, const char *se2)
{
	int r;

	if (!assertMakeDir(target, 0775))
	    return;

	/* Use the cpio program to create an archive. */
	r = systemf("%s -o %s < filelist >%s/archive 2>%s/pack.err",
	    testprog, pack_options, target, target);
	failure("Error invoking %s -o %s", testprog, pack_options);
	assertEqualInt(r, 0);

	assertChdir(target);

	/* Verify stderr. */
	failure("Expected: %s, options=%s", se, pack_options);
	assertTextFileContents(se, "pack.err");

	/*
	 * Use cpio to unpack the archive into another directory.
	 */
	r = systemf("%s -i %s< archive >unpack.out 2>unpack.err",
	    testprog, unpack_options);
	failure("Error invoking %s -i %s", testprog, unpack_options);
	assertEqualInt(r, 0);

	/* Verify stderr. */
	failure("Error invoking %s -i %s in dir %s", testprog, unpack_options, target);
	assertTextFileContents(se2, "unpack.err");

	verify_files(pack_options);

	assertChdir("..");
}

static void
passthrough(const char *target)
{
	int r;

	if (!assertMakeDir(target, 0775))
		return;

	/*
	 * Use cpio passthrough mode to copy files to another directory.
	 */
	r = systemf("%s -p %s <filelist >%s/stdout 2>%s/stderr",
	    testprog, target, target, target);
	failure("Error invoking %s -p", testprog);
	assertEqualInt(r, 0);

	assertChdir(target);

	/* Verify stderr. */
	failure("Error invoking %s -p in dir %s",
	    testprog, target);
	assertTextFileContents("1 block\n", "stderr");

	verify_files("passthrough");
	assertChdir("..");
}

DEFINE_TEST(test_basic)
{
	FILE *filelist;
	const char *msg;
	char result[1024];

	assertUmask(0);

	/*
	 * Create an assortment of files on disk.
	 */
	filelist = fopen("filelist", "w");
	memset(result, 0, sizeof(result));

	/* File with 10 bytes content. */
	assertMakeFile("file", 0644, "1234567890");
	fprintf(filelist, "file\n");
	if (is_LargeInode("file"))
		strncat(result,
		    "bsdcpio: file: large inode number truncated: "
		    "Numerical result out of range\n",
		    sizeof(result) - strlen(result) -1);

	/* hardlink to above file. */
	assertMakeHardlink("linkfile", "file");
	fprintf(filelist, "linkfile\n");
	if (is_LargeInode("linkfile"))
		strncat(result,
		    "bsdcpio: linkfile: large inode number truncated: "
		    "Numerical result out of range\n",
		    sizeof(result) - strlen(result) -1);

	/* Symlink to above file. */
	if (canSymlink()) {
		assertMakeSymlink("symlink", "file");
		fprintf(filelist, "symlink\n");
		if (is_LargeInode("symlink"))
			strncat(result,
			    "bsdcpio: symlink: large inode number truncated: "
				"Numerical result out of range\n",
			    sizeof(result) - strlen(result) -1);
	}

	/* Another file with different permissions. */
	assertMakeFile("file2", 0777, "1234567890");
	fprintf(filelist, "file2\n");
	if (is_LargeInode("file2"))
		strncat(result,
		    "bsdcpio: file2: large inode number truncated: "
		    "Numerical result out of range\n",
		    sizeof(result) - strlen(result) -1);

	/* Directory. */
	assertMakeDir("dir", 0775);
	fprintf(filelist, "dir\n");
	if (is_LargeInode("dir"))
		strncat(result,
		    "bsdcpio: dir: large inode number truncated: "
		    "Numerical result out of range\n",
		    sizeof(result) - strlen(result) -1);
	strncat(result, "2 blocks\n", sizeof(result) - strlen(result) -1);

	/* All done. */
	fclose(filelist);

	assertUmask(022);

	/* Archive/dearchive with a variety of options. */
	msg = canSymlink() ? "2 blocks\n" : "1 block\n";
	basic_cpio("copy", "", "", msg, msg);
	basic_cpio("copy_odc", "--format=odc", "", msg, msg);
	basic_cpio("copy_newc", "-H newc", "", result, "2 blocks\n");
	basic_cpio("copy_cpio", "-H odc", "", msg, msg);
	msg = canSymlink() ? "9 blocks\n" : "8 blocks\n";
	basic_cpio("copy_ustar", "-H ustar", "", msg, msg);

	/* Copy in one step using -p */
	passthrough("passthrough");
}