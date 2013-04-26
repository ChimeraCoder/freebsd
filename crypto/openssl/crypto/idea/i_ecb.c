
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

#include <openssl/idea.h>
#include "idea_lcl.h"
#include <openssl/opensslv.h>

const char IDEA_version[]="IDEA" OPENSSL_VERSION_PTEXT;

const char *idea_options(void)
	{
	if (sizeof(short) != sizeof(IDEA_INT))
		return("idea(int)");
	else
		return("idea(short)");
	}

void idea_ecb_encrypt(const unsigned char *in, unsigned char *out,
	     IDEA_KEY_SCHEDULE *ks)
	{
	unsigned long l0,l1,d[2];

	n2l(in,l0); d[0]=l0;
	n2l(in,l1); d[1]=l1;
	idea_encrypt(d,ks);
	l0=d[0]; l2n(l0,out);
	l1=d[1]; l2n(l1,out);
	l0=l1=d[0]=d[1]=0;
	}