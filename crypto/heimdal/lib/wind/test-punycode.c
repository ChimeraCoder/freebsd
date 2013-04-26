
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <string.h>

#include "windlocl.h"
#include "punycode_examples.h"

int
main(void)
{
    unsigned i;
    unsigned failures = 0;

    for (i = 0; i < punycode_examples_size; ++i) {
	char buf[256];
	int ret;
	const struct punycode_example *e = &punycode_examples[i];
	size_t len;

	len = sizeof(buf);
	ret = wind_punycode_label_toascii(e->val, e->len, buf, &len);
	if (ret) {
	    printf("punycode %u (%s) failed: %d\n", i, e->description, ret);
	    ++failures;
	    continue;
	}
	if (strncmp(buf, "xn--", 4) == 0) {
	    memmove(buf, buf + 4, len - 4);
	    len -= 4;
	}
	if (len != strlen(e->pc)) {
	    printf("punycode %u (%s) wrong len, actual: %u, expected: %u\n",
		   i, e->description,
		   (unsigned int)len, (unsigned int)strlen(e->pc));
	    printf("buf %s != pc: %s\n", buf, e->pc);
	    ++failures;
	    continue;
	}
	if (strncasecmp(buf, e->pc, len) != 0) {
	    printf("punycode %u (%s) wrong contents, "
		   "actual: \"%.*s\", expected: \"%s\"\n",
		   i, e->description, (unsigned int)len, buf, e->pc);
	    ++failures;
	    continue;
	}
    }
    return failures != 0;
}