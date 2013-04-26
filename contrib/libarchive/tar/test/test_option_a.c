
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

DEFINE_TEST(test_option_a)
{
	size_t s;
	char *p;

	/* Create a file. */
	assertMakeFile("f", 0644, "a");

	/* Test1: archive it with .tar.Z suffix. */
	assertEqualInt(0,
	    systemf("%s -acf test1.tar.Z f 2>test1.err", testprog));
	assertEmptyFile("test1.err");
	/* Check that the archive file has a compress signature. */
	p = slurpfile(&s, "test1.tar.Z");
	assert(s > 2);
	failure("The archive should be compressed");
	assertEqualMem(p, "\x1f\x9d", 2);

	/* Test2: archive it with .taZ suffix. */
	assertEqualInt(0,
	    systemf("%s -acf test2.taZ f 2>test2.err", testprog));
	assertEmptyFile("test2.err");
	/* Check that the archive file has a compress signature. */
	p = slurpfile(&s, "test2.taZ");
	assert(s > 2);
	failure("The archive should be compressed");
	assertEqualMem(p, "\x1f\x9d", 2);

	/* Test3: archive it with .tar.Z.uu suffix. */
	assertEqualInt(0,
	    systemf("%s -acf test3.tar.Z.uu f 2>test3.err", testprog));
	assertEmptyFile("test3.err");
	/* Check that the archive file has a compress signature. */
	p = slurpfile(&s, "test3.tar.Z.uu");
	assert(s > 12);
	failure("The archive should be uuencoded");
	assertEqualMem(p, "begin 644 -\n", 12);

	/* Test4: archive it with .zip suffix. */
	assertEqualInt(0,
	    systemf("%s -acf test4.zip f 2>test4.err", testprog));
	assertEmptyFile("test4.err");
	/* Check that the archive file has a compress signature. */
	p = slurpfile(&s, "test4.zip");
	assert(s > 4);
	failure("The archive should be zipped");
	assertEqualMem(p, "\x50\x4b\x03\x04", 4);

	/* Test5: archive it with .tar.Z suffix and --uuencode option. */
	assertEqualInt(0,
	    systemf("%s -acf test5.tar.Z --uuencode f 2>test5.err",
		testprog));
	assertEmptyFile("test5.err");
	/* Check that the archive file has a compress signature. */
	p = slurpfile(&s, "test5.tar.Z");
	assert(s > 2);
	failure("The archive should be compressed, ignoring --uuencode option");
	assertEqualMem(p, "\x1f\x9d", 2);

	/* Test6: archive it with .xxx suffix(unknown suffix) and
	 * --uuencode option. */
	assertEqualInt(0,
	    systemf("%s -acf test6.xxx --uuencode f 2>test6.err",
		testprog));
	assertEmptyFile("test6.err");
	/* Check that the archive file has a compress signature. */
	p = slurpfile(&s, "test6.xxx");
	assert(s > 12);
	failure("The archive should be uuencoded");
	assertEqualMem(p, "begin 644 -\n", 12);

	/* Test7: archive it with .tar.Z suffix using a long-name option. */
	assertEqualInt(0,
	    systemf("%s --auto-compress -cf test7.tar.Z f 2>test7.err",
		testprog));
	assertEmptyFile("test7.err");
	/* Check that the archive file has a compress signature. */
	p = slurpfile(&s, "test7.tar.Z");
	assert(s > 2);
	failure("The archive should be compressed");
	assertEqualMem(p, "\x1f\x9d", 2);
}