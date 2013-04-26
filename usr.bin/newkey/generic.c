
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

#if !defined(lint) && defined(SCCSIDS)
#if 0
static char sccsid[] = "@(#)generic.c 1.2 91/03/11 Copyr 1986 Sun Micro";
#endif
#endif

/*
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/file.h>

#include <rpc/rpc.h>
#include <rpc/key_prot.h>

#include <mp.h>
#include <stdio.h>
#include <stdlib.h>

#include "extern.h"

static void adjust(char[], char *);
static void getseed(char *, int, unsigned char *);

/*
 * Generate a seed
 */
static void
getseed(char *seed, int seedsize, unsigned char *pass)
{
	int i;

	for (i = 0; i < seedsize; i++) {
		seed[i] = (arc4random() & 0xff) ^ pass[i % 8];
	}
}

/*
 * Generate a random public/secret key pair
 */
void
genkeys(char *public, char *secret, char *pass)
{
	unsigned int i;

#   define BASEBITS (8*sizeof (short) - 1)
#	define BASE		(1 << BASEBITS)

	MINT *pk = mp_itom(0);
	MINT *sk = mp_itom(0);
	MINT *tmp;
	MINT *base = mp_itom(BASE);
	MINT *root = mp_itom(PROOT);
	MINT *modulus = mp_xtom(HEXMODULUS);
	short r;
	unsigned short seed[KEYSIZE/BASEBITS + 1];
	char *xkey;

	getseed((char *)seed, sizeof (seed), (u_char *)pass);
	for (i = 0; i < KEYSIZE/BASEBITS + 1; i++) {
		r = seed[i] % BASE;
		tmp = mp_itom(r);
		mp_mult(sk, base, sk);
		mp_madd(sk, tmp, sk);
		mp_mfree(tmp);
	}
	tmp = mp_itom(0);
	mp_mdiv(sk, modulus, tmp, sk);
	mp_mfree(tmp);
	mp_pow(root, sk, modulus, pk);
	xkey = mp_mtox(sk);
	adjust(secret, xkey);
	xkey = mp_mtox(pk);
	adjust(public, xkey);
	mp_mfree(sk);
	mp_mfree(base);
	mp_mfree(pk);
	mp_mfree(root);
	mp_mfree(modulus);
}

/*
 * Adjust the input key so that it is 0-filled on the left
 */
static void
adjust(char keyout[HEXKEYBYTES+1], char *keyin)
{
	char *p;
	char *s;

	for (p = keyin; *p; p++)
		;
	for (s = keyout + HEXKEYBYTES; p >= keyin; p--, s--) {
		*s = *p;
	}
	while (s >= keyout) {
		*s-- = '0';
	}
}