
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/lhash.h>

main()
	{
	LHASH *conf;
	char buf[256];
	int i;

	conf=lh_new(lh_strhash,strcmp);
	for (;;)
		{
		char *p;

		buf[0]='\0';
		fgets(buf,256,stdin);
		if (buf[0] == '\0') break;
		i=strlen(buf);
		p=OPENSSL_malloc(i+1);
		memcpy(p,buf,i+1);
		lh_insert(conf,p);
		}

	lh_node_stats(conf,stdout);
	lh_stats(conf,stdout);
	lh_node_usage_stats(conf,stdout);
	exit(0);
	}