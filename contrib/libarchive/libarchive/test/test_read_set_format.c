
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

DEFINE_TEST(test_read_set_format)
{
  char buff[64];
  const char reffile[] = "test_read_format_rar.rar";
  const char test_txt[] = "test text document\r\n";
  int size = sizeof(test_txt)-1;
  struct archive_entry *ae;
  struct archive *a;

  extract_reference_file(reffile);
  assert((a = archive_read_new()) != NULL);
  assertA(0 == archive_read_set_format(a, ARCHIVE_FORMAT_RAR));
  assertA(0 == archive_read_append_filter(a, ARCHIVE_FILTER_NONE));
  assertA(0 == archive_read_open_filename(a, reffile, 10240));

  /* First header. */
  assertA(0 == archive_read_next_header(a, &ae));
  assertEqualString("test.txt", archive_entry_pathname(ae));
  assertA((int)archive_entry_mtime(ae));
  assertA((int)archive_entry_ctime(ae));
  assertA((int)archive_entry_atime(ae));
  assertEqualInt(20, archive_entry_size(ae));
  assertEqualInt(33188, archive_entry_mode(ae));
  assertA(size == archive_read_data(a, buff, size));
  assertEqualMem(buff, test_txt, size);

  /* Second header. */
  assertA(0 == archive_read_next_header(a, &ae));
  assertEqualString("testlink", archive_entry_pathname(ae));
  assertA((int)archive_entry_mtime(ae));
  assertA((int)archive_entry_ctime(ae));
  assertA((int)archive_entry_atime(ae));
  assertEqualInt(0, archive_entry_size(ae));
  assertEqualInt(41471, archive_entry_mode(ae));
  assertEqualString("test.txt", archive_entry_symlink(ae));
  assertEqualIntA(a, 0, archive_read_data(a, buff, sizeof(buff)));

  /* Third header. */
  assertA(0 == archive_read_next_header(a, &ae));
  assertEqualString("testdir/test.txt", archive_entry_pathname(ae));
  assertA((int)archive_entry_mtime(ae));
  assertA((int)archive_entry_ctime(ae));
  assertA((int)archive_entry_atime(ae));
  assertEqualInt(20, archive_entry_size(ae));
  assertEqualInt(33188, archive_entry_mode(ae));
  assertA(size == archive_read_data(a, buff, size));
  assertEqualMem(buff, test_txt, size);

  /* Fourth header. */
  assertA(0 == archive_read_next_header(a, &ae));
  assertEqualString("testdir", archive_entry_pathname(ae));
  assertA((int)archive_entry_mtime(ae));
  assertA((int)archive_entry_ctime(ae));
  assertA((int)archive_entry_atime(ae));
  assertEqualInt(0, archive_entry_size(ae));
  assertEqualInt(16877, archive_entry_mode(ae));

  /* Fifth header. */
  assertA(0 == archive_read_next_header(a, &ae));
  assertEqualString("testemptydir", archive_entry_pathname(ae));
  assertA((int)archive_entry_mtime(ae));
  assertA((int)archive_entry_ctime(ae));
  assertA((int)archive_entry_atime(ae));
  assertEqualInt(0, archive_entry_size(ae));
  assertEqualInt(16877, archive_entry_mode(ae));

  /* Test EOF */
  assertA(1 == archive_read_next_header(a, &ae));
  assertEqualInt(5, archive_file_count(a));
  assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
  assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

DEFINE_TEST(test_read_set_wrong_format)
{
  const char reffile[] = "test_read_format_zip.zip";
  struct archive_entry *ae;
  struct archive *a;

  extract_reference_file(reffile);
  assert((a = archive_read_new()) != NULL);
  assertA(0 == archive_read_set_format(a, ARCHIVE_FORMAT_RAR));
  assertA(0 == archive_read_append_filter(a, ARCHIVE_FILTER_NONE));
  assertA(0 == archive_read_open_filename(a, reffile, 10240));

  /* Check that this actually fails, then close the archive. */
  assertA(archive_read_next_header(a, &ae) < (ARCHIVE_WARN));
  assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
  assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

static unsigned char archive[] = {
31,139,8,0,222,'C','p','C',0,3,211,'c',160,'=','0','0','0','0','7','5','U',
0,210,134,230,166,6,200,'4',28,'(',24,26,24,27,155,24,152,24,154,27,155,')',
24,24,26,152,154,25,'2','(',152,210,193,'m',12,165,197,'%',137,'E','@',167,
148,'d',230,226,'U','G','H',30,234,15,'8','=',10,'F',193,'(',24,5,131,28,
0,0,29,172,5,240,0,6,0,0};

DEFINE_TEST(test_read_append_filter)
{
  struct archive_entry *ae;
  struct archive *a;
  int r;

  assert((a = archive_read_new()) != NULL);
  assertA(0 == archive_read_set_format(a, ARCHIVE_FORMAT_TAR));
  r = archive_read_append_filter(a, ARCHIVE_FILTER_GZIP);
  if (r == ARCHIVE_WARN && !canGzip()) {
    skipping("gzip reading not fully supported on this platform");
    assertEqualInt(ARCHIVE_OK, archive_read_free(a));
    return;
  }
  assertEqualInt(ARCHIVE_OK,
      archive_read_open_memory(a, archive, sizeof(archive)));
  assertEqualInt(ARCHIVE_OK, archive_read_next_header(a, &ae));
  assertEqualInt(1, archive_file_count(a));
  assertEqualInt(archive_filter_code(a, 0), ARCHIVE_COMPRESSION_GZIP);
  assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_USTAR);
  assertEqualInt(ARCHIVE_OK, archive_read_close(a));
  assertEqualInt(ARCHIVE_OK,archive_read_free(a));
}

DEFINE_TEST(test_read_append_wrong_filter)
{
  struct archive_entry *ae;
  struct archive *a;
  int r;

  assert((a = archive_read_new()) != NULL);
  assertA(0 == archive_read_set_format(a, ARCHIVE_FORMAT_TAR));
  r = archive_read_append_filter(a, ARCHIVE_FILTER_XZ);
  if (r == ARCHIVE_WARN && !canXz()) {
    skipping("xz reading not fully supported on this platform");
    assertEqualInt(ARCHIVE_OK, archive_read_free(a));
    return;
  }
  assertEqualInt(ARCHIVE_OK,
      archive_read_open_memory(a, archive, sizeof(archive)));
  assertA(archive_read_next_header(a, &ae) < (ARCHIVE_WARN));
  if (r == ARCHIVE_WARN && canXz()) {
    assertEqualIntA(a, ARCHIVE_WARN, archive_read_close(a));
  } else {
    assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
  }
  assertEqualInt(ARCHIVE_OK,archive_read_free(a));
}

DEFINE_TEST(test_read_append_filter_program)
{
  struct archive_entry *ae;
  struct archive *a;

  if (!canGzip()) {
    skipping("Can't run gzip program on this platform");
    return;
  }
  assert((a = archive_read_new()) != NULL);
  assertA(0 == archive_read_set_format(a, ARCHIVE_FORMAT_TAR));
  assertEqualIntA(a, ARCHIVE_OK,
      archive_read_append_filter_program(a, "gzip -d"));
  assertEqualIntA(a, ARCHIVE_OK,
      archive_read_open_memory(a, archive, sizeof(archive)));
  assertEqualIntA(a, ARCHIVE_OK,
      archive_read_next_header(a, &ae));
  assertEqualInt(archive_filter_code(a, 0), ARCHIVE_FILTER_PROGRAM);
  assertEqualInt(archive_format(a), ARCHIVE_FORMAT_TAR_USTAR);
  assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
  assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

DEFINE_TEST(test_read_append_filter_wrong_program)
{
  struct archive_entry *ae;
  struct archive *a;

  /*
   * If we have "bunzip2 -q", try using that.
   */
  if (!canRunCommand("bunzip2 -V")) {
    skipping("Can't run bunzip2 program on this platform");
    return;
  }
  assert((a = archive_read_new()) != NULL);
  assertA(0 == archive_read_set_format(a, ARCHIVE_FORMAT_TAR));
  assertEqualIntA(a, ARCHIVE_OK,
      archive_read_append_filter_program(a, "bunzip2 -q"));
  assertEqualIntA(a, ARCHIVE_OK,
      archive_read_open_memory(a, archive, sizeof(archive)));
  assertA(archive_read_next_header(a, &ae) < (ARCHIVE_WARN));
  assertEqualIntA(a, ARCHIVE_WARN, archive_read_close(a));
  assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}