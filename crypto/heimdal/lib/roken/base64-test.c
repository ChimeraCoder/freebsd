
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
#include <base64.h>

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
	{ "1", 1, "MQ==" },
	{ "22", 2, "MjI=" },
	{ "333", 3, "MzMz" },
	{ "4444", 4, "NDQ0NA==" },
	{ "55555", 5, "NTU1NTU=" },
	{ "abc:def", 7, "YWJjOmRlZg==" },
	{ NULL }
    };
    for(t = tests; t->data; t++) {
	char *str;
	int len;
	len = base64_encode(t->data, t->len, &str);
	if(strcmp(str, t->result) != 0) {
	    fprintf(stderr, "failed test %d: %s != %s\n", numtest,
		    str, t->result);
	    numerr++;
	}
	free(str);
	str = strdup(t->result);
	len = base64_decode(t->result, str);
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
	char str[32];
	if(base64_decode("M=M=", str) != -1) {
	    fprintf(stderr, "failed test %d: successful decode of `M=M='\n",
		    numtest++);
	    numerr++;
	}
	if(base64_decode("MQ===", str) != -1) {
	    fprintf(stderr, "failed test %d: successful decode of `MQ==='\n",
		    numtest++);
	    numerr++;
	}
    }
    return numerr;
}