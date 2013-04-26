
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

#include "rpc_des.h"
#include "des_locl.h"
#include "des_ver.h"

int _des_crypt(char *buf,int len,struct desparams *desp);
int _des_crypt(char *buf, int len, struct desparams *desp)
	{
	DES_key_schedule ks;
	int enc;

	DES_set_key_unchecked(&desp->des_key,&ks);
	enc=(desp->des_dir == ENCRYPT)?DES_ENCRYPT:DES_DECRYPT;

	if (desp->des_mode == CBC)
		DES_ecb_encrypt((const_DES_cblock *)desp->UDES.UDES_buf,
				(DES_cblock *)desp->UDES.UDES_buf,&ks,
				enc);
	else
		{
		DES_ncbc_encrypt(desp->UDES.UDES_buf,desp->UDES.UDES_buf,
				len,&ks,&desp->des_ivec,enc);
#ifdef undef
		/* len will always be %8 if called from common_crypt
		 * in secure_rpc.
		 * Libdes's cbc encrypt does not copy back the iv,
		 * so we have to do it here. */
		/* It does now :-) eay 20/09/95 */

		a=(char *)&(desp->UDES.UDES_buf[len-8]);
		b=(char *)&(desp->des_ivec[0]);

		*(a++)= *(b++); *(a++)= *(b++);
		*(a++)= *(b++); *(a++)= *(b++);
		*(a++)= *(b++); *(a++)= *(b++);
		*(a++)= *(b++); *(a++)= *(b++);
#endif
		}
	return(1);	
	}