
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
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/*
 * Test UFS file flags with/without use-set option.
 */
#if defined(UF_IMMUTABLE) && defined(UF_NODUMP)

static char buff[4096];
static struct {
	const char	*path;
	unsigned long	 fflags;
} entries[] = {
	{ "./f1", 	UF_IMMUTABLE | UF_NODUMP },
	{ "./f11", 	UF_IMMUTABLE | UF_NODUMP },
	{ "./f2", 	0 },
	{ "./f3", 	UF_NODUMP },
	{ NULL, 0 }
};

static void
test_write_format_mtree_sub(int use_set)
{
	struct archive_entry *ae;
	struct archive* a;
	size_t used;
	int i;

	/* Create a mtree format archive. */
	assert((a = archive_write_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_write_set_format_mtree(a));
	if (use_set)
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_set_options(a, "use-set,!all,flags,type"));
	else
		assertEqualIntA(a, ARCHIVE_OK,
		    archive_write_set_options(a, "!all,flags,type"));
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_write_open_memory(a, buff, sizeof(buff)-1, &used));

	/* Write entries */
	for (i = 0; entries[i].path != NULL; i++) {
		assert((ae = archive_entry_new()) != NULL);
		archive_entry_set_fflags(ae, entries[i].fflags, 0);
		archive_entry_copy_pathname(ae, entries[i].path);
		archive_entry_set_size(ae, 0);
		assertEqualIntA(a, ARCHIVE_OK, archive_write_header(a, ae));
		archive_entry_free(ae);
	}
	assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
        assertEqualInt(ARCHIVE_OK, archive_write_free(a));

	if (use_set) {
		const char *p;

		buff[used] = '\0';
		assert(NULL != (p = strstr(buff, "\n/set ")));
		if (p != NULL) {
			char *r;
			const char *o;
			p++;
			r = strchr(p, '\n');
			if (r != NULL)
				*r = '\0';
			o = "/set type=file flags=uchg,nodump";
			assertEqualString(o, p);
			if (r != NULL)
				*r = '\n';
		}
	}

	/*
	 * Read the data and check it.
	 */
	assert((a = archive_read_new()) != NULL);
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_format_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_support_filter_all(a));
	assertEqualIntA(a, ARCHIVE_OK, archive_read_open_memory(a, buff, used));

	/* Read entries */
	for (i = 0; entries[i].path != NULL; i++) {
		unsigned long fset, fclr;

		assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
		archive_entry_fflags(ae, &fset, &fclr);
		assertEqualInt((int)entries[i].fflags, (int)fset);
		assertEqualInt(0, (int)fclr);
		assertEqualString(entries[i].path, archive_entry_pathname(ae));
	}
	assertEqualIntA(a, ARCHIVE_OK, archive_read_close(a));
	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

#endif

DEFINE_TEST(test_write_format_mtree_fflags)
{
#if defined(UF_IMMUTABLE) && defined(UF_NODUMP)
	/* Default setting */
	test_write_format_mtree_sub(0);
	/* Use /set keyword */
	test_write_format_mtree_sub(1);
#else
	skipping("This platform does not support UFS file flags");
#endif
}