
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


/* This file contains deprecated functions as wrappers to the new ones */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/bn.h>
#include <openssl/dh.h>

static void *dummy=&dummy;

#ifndef OPENSSL_NO_DEPRECATED
DH *DH_generate_parameters(int prime_len, int generator,
	     void (*callback)(int,int,void *), void *cb_arg)
	{
	BN_GENCB cb;
	DH *ret=NULL;

	if((ret=DH_new()) == NULL)
		return NULL;

	BN_GENCB_set_old(&cb, callback, cb_arg);

	if(DH_generate_parameters_ex(ret, prime_len, generator, &cb))
		return ret;
	DH_free(ret);
	return NULL;
	}
#endif