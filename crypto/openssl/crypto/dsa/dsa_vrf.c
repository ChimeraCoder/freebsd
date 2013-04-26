
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

/* Original version from Steven Schoch <schoch@sheba.arc.nasa.gov> */

#include "cryptlib.h"
#include <openssl/dsa.h>

int DSA_do_verify(const unsigned char *dgst, int dgst_len, DSA_SIG *sig,
		  DSA *dsa)
	{
#ifdef OPENSSL_FIPS
	if (FIPS_mode() && !(dsa->meth->flags & DSA_FLAG_FIPS_METHOD)
			&& !(dsa->flags & DSA_FLAG_NON_FIPS_ALLOW))
		{
		DSAerr(DSA_F_DSA_DO_VERIFY, DSA_R_NON_FIPS_DSA_METHOD);
		return -1;
		}
#endif
	return dsa->meth->dsa_do_verify(dgst, dgst_len, sig, dsa);
	}