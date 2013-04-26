
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

#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <dlfcn.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/des_crypt.h>
#include <rpc/des.h>
#include "crypt.h"

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

/*
 * The U.S. government stupidly believes that a) it can keep strong
 * crypto code a secret and b) that doing so somehow protects national
 * interests. It's wrong on both counts, but until it listens to reason
 * we have to make certain compromises so it doesn't have an excuse to
 * throw us in federal prison.
 *
 * Consequently, the core OS ships without DES support, and keyserv
 * defaults to using ARCFOUR with only a 40 bit key, just like nutscrape.
 * This breaks compatibility with Secure RPC on other systems, but it
 * allows Secure RPC to work between FreeBSD systems that don't have the
 * DES package installed without throwing security totally out the window.
 *
 * In order to avoid having to supply two versions of keyserv (one with
 * DES and one without), we use dlopen() and friends to load libdes.so
 * into our address space at runtime. We check for the presence of
 * /usr/lib/libdes.so.3.0 at startup and load it if we find it. If we
 * can't find it, or the __des_crypt symbol doesn't exist, we fall back
 * to the ARCFOUR encryption code. The user can specify another path using
 * the -p flag.
 */

 /* arcfour.h */
typedef struct arcfour_key
{      
   unsigned char state[256];       
   unsigned char x;        
   unsigned char y;
} arcfour_key;

static void prepare_key(unsigned char *key_data_ptr,int key_data_len,
		 arcfour_key *key);
static void arcfour(unsigned char *buffer_ptr,int buffer_len,arcfour_key * key);
static void swap_byte(unsigned char *a, unsigned char *b);

static void prepare_key(unsigned char *key_data_ptr, int key_data_len,
		 arcfour_key *key)
{
   unsigned char index1;
   unsigned char index2;
   unsigned char* state;
   short counter;     

   state = &key->state[0];         
   for(counter = 0; counter < 256; counter++)              
   state[counter] = counter;               
   key->x = 0;     
   key->y = 0;     
   index1 = 0;     
   index2 = 0;             
   for(counter = 0; counter < 256; counter++)      
   {               
      index2 = (key_data_ptr[index1] + state[counter] +
                index2) % 256;                
      swap_byte(&state[counter], &state[index2]);            
      
      index1 = (index1 + 1) % key_data_len;  
   }       
}

static void arcfour(unsigned char *buffer_ptr, int buffer_len, arcfour_key *key)
{ 
   unsigned char x;
   unsigned char y;
   unsigned char* state;
   unsigned char xorIndex;
   short counter;              
   
   x = key->x;     
   y = key->y;     
   
   state = &key->state[0];         
   for(counter = 0; counter < buffer_len; counter ++)      
   {               
      x = (x + 1) % 256;                      
      y = (state[x] + y) % 256;               
      swap_byte(&state[x], &state[y]);                        
      
      xorIndex = (state[x] + state[y]) % 256;                 
      
      buffer_ptr[counter] ^= state[xorIndex];         
   }               
   key->x = x;     
   key->y = y;
}

static void swap_byte(unsigned char *a, unsigned char *b)
{
   unsigned char swapByte; 
   
   swapByte = *a; 
   *a = *b;      
   *b = swapByte;
}

/* Dummy _des_crypt function that uses ARCFOUR with a 40 bit key */
int _arcfour_crypt(buf, len, desp)
	char *buf;
	int len;
	struct desparams *desp;
{
	struct arcfour_key arcfourk;

	/*
	 * U.S. government anti-crypto weasels take
	 * note: although we are supplied with a 64 bit
	 * key, we're only passing 40 bits to the ARCFOUR
	 * encryption code. So there.
	 */
	prepare_key(desp->des_key, 5, &arcfourk);
	arcfour(buf, len, &arcfourk);

	return(DESERR_NOHWDEVICE);
}

int (*_my_crypt)(char *, int, struct desparams *) = NULL;

static void *dlhandle;

#ifndef _PATH_USRLIB
#define _PATH_USRLIB "/usr/lib"
#endif

#ifndef LIBCRYPTO
#define LIBCRYPTO "libcrypto.so.2"
#endif

void load_des(warn, libpath)
	int warn;
	char *libpath;
{
	char dlpath[MAXPATHLEN];

	if (libpath == NULL) {
		snprintf(dlpath, sizeof(dlpath), "%s/%s", _PATH_USRLIB, LIBCRYPTO);
	} else
		snprintf(dlpath, sizeof(dlpath), "%s", libpath);

	if (dlpath != NULL && (dlhandle = dlopen(dlpath, 0444)) != NULL)
		_my_crypt = (int (*)())dlsym(dlhandle, "_des_crypt");

	if (_my_crypt == NULL) {
		if (dlhandle != NULL)
			dlclose(dlhandle);
		_my_crypt = &_arcfour_crypt;
		if (warn) {
			printf ("DES support disabled -- using ARCFOUR instead.\n");
			printf ("Warning: ARCFOUR cipher is not compatible with ");
			printf ("other Secure RPC implementations.\nInstall ");
			printf ("the FreeBSD 'des' distribution to enable");
			printf (" DES encryption.\n");
		}
	} else {
		if (warn) {
			printf ("DES support enabled\n");
			printf ("Using %s shared object.\n", dlpath);
		}
	}

	return;
}

desresp *
des_crypt_1_svc(desargs *argp, struct svc_req *rqstp)
{
	static desresp  result;
	struct desparams dparm;

	if (argp->desbuf.desbuf_len > DES_MAXDATA) {
		result.stat = DESERR_BADPARAM;
		return(&result);
	}


	bcopy(argp->des_key, dparm.des_key, 8);
	bcopy(argp->des_ivec, dparm.des_ivec, 8);
	dparm.des_mode = (argp->des_mode == CBC_DES) ? CBC : ECB;
	dparm.des_dir = (argp->des_dir == ENCRYPT_DES) ? ENCRYPT : DECRYPT;
#ifdef BROKEN_DES
	dparm.UDES.UDES_buf = argp->desbuf.desbuf_val;
#endif

	/*
	 * XXX This compensates for a bug in the libdes Secure RPC
	 * compat interface. (Actually, there are a couple.) The
	 * des_ecb_encrypt() routine in libdes only encrypts 8 bytes
	 * (64 bits) at a time. However, the Sun Secure RPC ecb_crypt()
	 * routine is supposed to be able to handle buffers up to 8Kbytes.
	 * The rpc_enc module in libdes ignores this fact and just drops
	 * the length parameter on the floor, encrypting only the
	 * first 64 bits of whatever buffer you feed it. We deal with
	 * this here: if we're using DES encryption, and we're using
	 * ECB mode, then we make a pass over the entire buffer
	 * ourselves. Note: the rpc_enc module incorrectly transposes
	 * the mode flags, so when you ask for CBC mode, you're really
	 * getting ECB mode.
	 */
#ifdef BROKEN_DES
	if (_my_crypt != &_arcfour_crypt && argp->des_mode == CBC) {
#else
	if (_my_crypt != &_arcfour_crypt && argp->des_mode == ECB) {
#endif
		int			i;
		char			*dptr;

		for (i = 0; i < argp->desbuf.desbuf_len / 8; i++) {
			dptr = argp->desbuf.desbuf_val;
			dptr += (i * 8);
#ifdef BROKEN_DES
			dparm.UDES.UDES_buf = dptr;
#endif
			result.stat = _my_crypt(dptr, 8, &dparm);
		}
	} else {
		result.stat = _my_crypt(argp->desbuf.desbuf_val,
					argp->desbuf.desbuf_len,
					&dparm);
	}

	if (result.stat == DESERR_NONE || result.stat == DESERR_NOHWDEVICE) {
		bcopy(dparm.des_ivec, result.des_ivec, 8);
		result.desbuf.desbuf_len = argp->desbuf.desbuf_len;
		result.desbuf.desbuf_val = argp->desbuf.desbuf_val;
	}

	return (&result);
}