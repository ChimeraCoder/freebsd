
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
Execute the following command to rebuild the data for this program:
   tail -n +32 test_read_format_isorr_rr_moved.c | /bin/sh

dirname=/tmp/iso
rm -rf $dirname
mkdir -p $dirname/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/dir10
echo "hello" >$dirname/file
dd if=/dev/zero count=1 bs=12345678 >>$dirname/file
deepfile=$dirname/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/dir10/deep
echo "hello" >$deepfile
dd if=/dev/zero count=1 bs=12345678 >>$deepfile
time="197001020000.01"
TZ=utc touch -afhm -t $time $deepfile
TZ=utc touch -afhm -t $time $dirname/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9/dir10
TZ=utc touch -afhm -t $time $dirname/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8/dir9
TZ=utc touch -afhm -t $time $dirname/dir1/dir2/dir3/dir4/dir5/dir6/dir7/dir8
TZ=utc touch -afhm -t $time $dirname/dir1/dir2/dir3/dir4/dir5/dir6/dir7
TZ=utc touch -afhm -t $time $dirname/dir1/dir2/dir3/dir4/dir5/dir6
TZ=utc touch -afhm -t $time $dirname/dir1/dir2/dir3/dir4/dir5
TZ=utc touch -afhm -t $time $dirname/dir1/dir2/dir3/dir4
TZ=utc touch -afhm -t $time $dirname/dir1/dir2/dir3
TZ=utc touch -afhm -t $time $dirname/dir1/dir2
TZ=utc touch -afhm -t $time $dirname/dir1
TZ=utc touch -afhm -t $time $dirname/file
TZ=utc touch -afhm -t $time $dirname
F=test_read_format_isorr_rockridge_moved.iso.Z
mkhybrid -R -uid 1 -gid 2 $dirname | compress > $F
uuencode $F $F > $F.uu
exit 1
 */

DEFINE_TEST(test_read_format_isorr_rr_moved)
{
	const char *refname = "test_read_format_iso_rockridge_rr_moved.iso.Z";
	struct archive_entry *ae;
	struct archive *a;
	const void *p;
	size_t size;
	int64_t offset;
	int i;

	extract_reference_file(refname);
	assert((a = archive_read_new()) != NULL);
	assertEqualInt(0, archive_read_support_filter_all(a));
	assertEqualInt(0, archive_read_support_format_all(a));
	assertEqualInt(ARCHIVE_OK,
	    archive_read_open_filename(a, refname, 10240));

	/* Retrieve each of the 8 files on the ISO image and
	 * verify that each one is what we expect. */
	for (i = 0; i < 13; ++i) {
		assertEqualInt(0, archive_read_next_header(a, &ae));

		if (strcmp(".", archive_entry_pathname(ae)) == 0) {
			/* '.' root directory. */
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			/* Now, we read timestamp recorded by RRIP "TF". */
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(0, archive_entry_mtime_nsec(ae));
			/* Now, we read links recorded by RRIP "PX". */
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualIntA(a, ARCHIVE_EOF,
			    archive_read_data_block(a, &p, &size, &offset));
			assertEqualInt((int)size, 0);
		} else if (strcmp("dir1", archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1", archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2",
		    archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1/dir2",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2/dir3",
		    archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1/dir2/dir3",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2/dir3/dir4",
		    archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1/dir2/dir3/dir4",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2/dir3/dir4/dir5",
		    archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1/dir2/dir3/dir4/dir5",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2/dir3/dir4/dir5/dir6",
		    archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1/dir2/dir3/dir4/dir5/dir6",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2/dir3/dir4/dir5/dir6/dir7",
		    archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1/dir2/dir3/dir4/dir5/dir6/dir7",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2/dir3/dir4/dir5/dir6/dir7"
		   "/dir8",
		    archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1/dir2/dir3/dir4/dir5/dir6/dir7"
			    "/dir8",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2/dir3/dir4/dir5/dir6/dir7"
		   "/dir8/dir9",
		    archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1/dir2/dir3/dir4/dir5/dir6/dir7"
			    "/dir8/dir9",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(3, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2/dir3/dir4/dir5/dir6/dir7"
		   "/dir8/dir9/dir10",
		    archive_entry_pathname(ae)) == 0) {
			/* A directory. */
			assertEqualString("dir1/dir2/dir3/dir4/dir5/dir6/dir7"
			    "/dir8/dir9/dir10",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));
			assertEqualInt(2048, archive_entry_size(ae));
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(2, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("file", archive_entry_pathname(ae)) == 0) {
			/* A regular file. */
			assertEqualString("file", archive_entry_pathname(ae));
			assertEqualInt(AE_IFREG, archive_entry_filetype(ae));
			assertEqualInt(12345684, archive_entry_size(ae));
			assertEqualInt(0,
			    archive_read_data_block(a, &p, &size, &offset));
			assertEqualInt(0, offset);
			assertEqualMem(p, "hello\n", 6);
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(1, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else if (strcmp("dir1/dir2/dir3/dir4/dir5/dir6/dir7"
		    "/dir8/dir9/dir10/deep",
		    archive_entry_pathname(ae)) == 0) {
			/* A regular file. */
			assertEqualString("dir1/dir2/dir3/dir4/dir5/dir6/dir7"
			    "/dir8/dir9/dir10/deep",
			    archive_entry_pathname(ae));
			assertEqualInt(AE_IFREG, archive_entry_filetype(ae));
			assertEqualInt(12345684, archive_entry_size(ae));
			assertEqualInt(0,
			    archive_read_data_block(a, &p, &size, &offset));
			assertEqualInt(0, offset);
			assertEqualMem(p, "hello\n", 6);
			assertEqualInt(86401, archive_entry_mtime(ae));
			assertEqualInt(86401, archive_entry_atime(ae));
			assertEqualInt(1, archive_entry_stat(ae)->st_nlink);
			assertEqualInt(1, archive_entry_uid(ae));
			assertEqualInt(2, archive_entry_gid(ae));
		} else {
			failure("Saw a file that shouldn't have been there");
			assertEqualString(archive_entry_pathname(ae), "");
		}
	}

	/* End of archive. */
	assertEqualInt(ARCHIVE_EOF, archive_read_next_header(a, &ae));

	/* Verify archive format. */
	assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_COMPRESS);
	assertEqualInt(archive_format(a), ARCHIVE_FORMAT_ISO9660_ROCKRIDGE);

	/* Close the archive. */
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}