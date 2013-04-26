
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

#include "ipf.h"

alist_t *     
alist_new(int v, char *host)
{
	int a, b, c, d, bits;
	char *slash;    
	alist_t *al;  
	u_int mask;     

	al = calloc(1, sizeof(*al));
	if (al == NULL) {
		fprintf(stderr, "alist_new out of memory\n");
		return NULL;
	}

	bits = -1;      
	slash = strchr(host, '/');
	if (slash != NULL) {
		*slash = '\0';
		bits = atoi(slash + 1);
	}

	a = b = c = d = -1;
	sscanf(host, "%d.%d.%d.%d", &a, &b, &c, &d);

	if (bits > 0 && bits < 33) {
		mask = 0xffffffff << (32 - bits);
	} else if (b == -1) {
		mask = 0xff000000;
		b = c = d = 0;  
	} else if (c == -1) {
		mask = 0xffff0000;
		c = d = 0;
	} else if (d == -1) {
		mask = 0xffffff00;
		d = 0;
	} else {
		mask = 0xffffffff;
	}

	if (*host == '!') {
		al->al_not = 1;
		host++;
	}

	if (gethost(host, &al->al_addr) == -1) {
		if (slash != NULL)
			*slash = '/';
		fprintf(stderr, "Cannot parse hostname\n");
		free(al);
		return NULL;
	}
	al->al_mask = htonl(mask);
	if (slash != NULL)
		*slash = '/';
	return al;
}