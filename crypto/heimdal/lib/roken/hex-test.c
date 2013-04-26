
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

#include <config.h>

#include "roken.h"
#include <hex.h>

int
main(int argc, char **argv)
{
    int numerr = 0;
    int numtest = 1;
    struct test {
	void *data;
	size_t len;
	const char *result;
    } *t, tests[] = {
	{ "", 0 , "" },
	{ "a", 1, "61" },
	{ "ab", 2, "6162" },
	{ "abc", 3, "616263" },
	{ "abcd", 4, "61626364" },
	{ "abcde", 5, "6162636465" },
	{ "abcdef", 6, "616263646566" },
	{ "abcdefg", 7, "61626364656667" },
	{ "=", 1, "3D" },
	{ NULL }
    };
    for(t = tests; t->data; t++) {
	char *str;
	int len;
	len = hex_encode(t->data, t->len, &str);
	if(strcmp(str, t->result) != 0) {
	    fprintf(stderr, "failed test %d: %s != %s\n", numtest,
		    str, t->result);
	    numerr++;
	}
	free(str);
	str = strdup(t->result);
	len = strlen(str);
	len = hex_decode(t->result, str, len);
	if(len != t->len) {
	    fprintf(stderr, "failed test %d: len %lu != %lu\n", numtest,
		    (unsigned long)len, (unsigned long)t->len);
	    numerr++;
	} else if(memcmp(str, t->data, t->len) != 0) {
	    fprintf(stderr, "failed test %d: data\n", numtest);
	    numerr++;
	}
	free(str);
	numtest++;
    }

    {
	unsigned char buf[2] = { 0, 0xff } ;
	int len;

	len = hex_decode("A", buf, 1);
	if (len != 1) {
	    fprintf(stderr, "len != 1");
	    numerr++;
	}
	if (buf[0] != 10) {
	    fprintf(stderr, "buf != 10");
	    numerr++;
	}
	if (buf[1] != 0xff) {
	    fprintf(stderr, "buf != 0xff");
	    numerr++;
	}

    }

    return numerr;
}