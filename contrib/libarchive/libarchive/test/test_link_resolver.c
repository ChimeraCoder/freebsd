
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

static void test_linkify_tar(void)
{
	struct archive_entry *entry, *e2;
	struct archive_entry_linkresolver *resolver;

	/* Initialize the resolver. */
	assert(NULL != (resolver = archive_entry_linkresolver_new()));
	archive_entry_linkresolver_set_strategy(resolver,
	    ARCHIVE_FORMAT_TAR_USTAR);

	/* Create an entry with only 1 link and try to linkify it. */
	assert(NULL != (entry = archive_entry_new()));
	archive_entry_set_pathname(entry, "test1");
	archive_entry_set_ino(entry, 1);
	archive_entry_set_dev(entry, 2);
	archive_entry_set_nlink(entry, 1);
	archive_entry_set_size(entry, 10);
	archive_entry_linkify(resolver, &entry, &e2);

	/* Shouldn't have been changed. */
	assert(e2 == NULL);
	assertEqualInt(10, archive_entry_size(entry));
	assertEqualString("test1", archive_entry_pathname(entry));

	/* Now, try again with an entry that has 2 links. */
	archive_entry_set_pathname(entry, "test2");
	archive_entry_set_nlink(entry, 2);
	archive_entry_set_ino(entry, 2);
	archive_entry_linkify(resolver, &entry, &e2);
	/* Shouldn't be altered, since it wasn't seen before. */
	assert(e2 == NULL);
	assertEqualString("test2", archive_entry_pathname(entry));
	assertEqualString(NULL, archive_entry_hardlink(entry));
	assertEqualInt(10, archive_entry_size(entry));

	/* Match again and make sure it does get altered. */
	archive_entry_linkify(resolver, &entry, &e2);
	assert(e2 == NULL);
	assertEqualString("test2", archive_entry_pathname(entry));
	assertEqualString("test2", archive_entry_hardlink(entry));
	assertEqualInt(0, archive_entry_size(entry));


	/* Dirs should never be matched as hardlinks, regardless. */
	archive_entry_set_pathname(entry, "test3");
	archive_entry_set_nlink(entry, 2);
	archive_entry_set_filetype(entry, AE_IFDIR);
	archive_entry_set_ino(entry, 3);
	archive_entry_set_hardlink(entry, NULL);
	archive_entry_linkify(resolver, &entry, &e2);
	/* Shouldn't be altered, since it wasn't seen before. */
	assert(e2 == NULL);
	assertEqualString("test3", archive_entry_pathname(entry));
	assertEqualString(NULL, archive_entry_hardlink(entry));

	/* Dir, so it shouldn't get matched. */
	archive_entry_linkify(resolver, &entry, &e2);
	assert(e2 == NULL);
	assertEqualString("test3", archive_entry_pathname(entry));
	assertEqualString(NULL, archive_entry_hardlink(entry));

	archive_entry_free(entry);
	archive_entry_linkresolver_free(resolver);
}

static void test_linkify_old_cpio(void)
{
	struct archive_entry *entry, *e2;
	struct archive_entry_linkresolver *resolver;

	/* Initialize the resolver. */
	assert(NULL != (resolver = archive_entry_linkresolver_new()));
	archive_entry_linkresolver_set_strategy(resolver,
	    ARCHIVE_FORMAT_CPIO_POSIX);

	/* Create an entry with 2 link and try to linkify it. */
	assert(NULL != (entry = archive_entry_new()));
	archive_entry_set_pathname(entry, "test1");
	archive_entry_set_ino(entry, 1);
	archive_entry_set_dev(entry, 2);
	archive_entry_set_nlink(entry, 2);
	archive_entry_set_size(entry, 10);
	archive_entry_linkify(resolver, &entry, &e2);

	/* Shouldn't have been changed. */
	assert(e2 == NULL);
	assertEqualInt(10, archive_entry_size(entry));
	assertEqualString("test1", archive_entry_pathname(entry));

	/* Still shouldn't be matched. */
	archive_entry_linkify(resolver, &entry, &e2);
	assert(e2 == NULL);
	assertEqualString("test1", archive_entry_pathname(entry));
	assertEqualString(NULL, archive_entry_hardlink(entry));
	assertEqualInt(10, archive_entry_size(entry));

	archive_entry_free(entry);
	archive_entry_linkresolver_free(resolver);
}

static void test_linkify_new_cpio(void)
{
	struct archive_entry *entry, *e2;
	struct archive_entry_linkresolver *resolver;

	/* Initialize the resolver. */
	assert(NULL != (resolver = archive_entry_linkresolver_new()));
	archive_entry_linkresolver_set_strategy(resolver,
	    ARCHIVE_FORMAT_CPIO_SVR4_NOCRC);

	/* Create an entry with only 1 link and try to linkify it. */
	assert(NULL != (entry = archive_entry_new()));
	archive_entry_set_pathname(entry, "test1");
	archive_entry_set_ino(entry, 1);
	archive_entry_set_dev(entry, 2);
	archive_entry_set_nlink(entry, 1);
	archive_entry_set_size(entry, 10);
	archive_entry_linkify(resolver, &entry, &e2);

	/* Shouldn't have been changed. */
	assert(e2 == NULL);
	assertEqualInt(10, archive_entry_size(entry));
	assertEqualString("test1", archive_entry_pathname(entry));

	/* Now, try again with an entry that has 3 links. */
	archive_entry_set_pathname(entry, "test2");
	archive_entry_set_nlink(entry, 3);
	archive_entry_set_ino(entry, 2);
	archive_entry_linkify(resolver, &entry, &e2);

	/* First time, it just gets swallowed. */
	assert(entry == NULL);
	assert(e2 == NULL);

	/* Match again. */
	assert(NULL != (entry = archive_entry_new()));
	archive_entry_set_pathname(entry, "test3");
	archive_entry_set_ino(entry, 2);
	archive_entry_set_dev(entry, 2);
	archive_entry_set_nlink(entry, 2);
	archive_entry_set_size(entry, 10);
	archive_entry_linkify(resolver, &entry, &e2);

	/* Should get back "test2" and nothing else. */
	assertEqualString("test2", archive_entry_pathname(entry));
	assertEqualInt(0, archive_entry_size(entry));
	archive_entry_free(entry);
	assert(NULL == e2);
	archive_entry_free(e2); /* This should be a no-op. */

	/* Match a third time. */
	assert(NULL != (entry = archive_entry_new()));
	archive_entry_set_pathname(entry, "test4");
	archive_entry_set_ino(entry, 2);
	archive_entry_set_dev(entry, 2);
	archive_entry_set_nlink(entry, 3);
	archive_entry_set_size(entry, 10);
	archive_entry_linkify(resolver, &entry, &e2);

	/* Should get back "test3". */
	assertEqualString("test3", archive_entry_pathname(entry));
	assertEqualInt(0, archive_entry_size(entry));

	/* Since "test4" was the last link, should get it back also. */
	assertEqualString("test4", archive_entry_pathname(e2));
	assertEqualInt(10, archive_entry_size(e2));

	archive_entry_free(entry);
	archive_entry_free(e2);
	archive_entry_linkresolver_free(resolver);
}

DEFINE_TEST(test_link_resolver)
{
	test_linkify_tar();
	test_linkify_old_cpio();
	test_linkify_new_cpio();
}