
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

#include <openssl/opensslconf.h>

#include <stdio.h>
#include <string.h>
#include <openssl/engine.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#if (defined(__i386)   || defined(__i386__)   || defined(_M_IX86) || \
     defined(__x86_64) || defined(__x86_64__) || \
     defined(_M_AMD64) || defined (_M_X64)) && defined(OPENSSL_CPUID_OBJ)

size_t OPENSSL_ia32_rdrand(void);

static int get_random_bytes (unsigned char *buf, int num)
	{
	size_t rnd;

	while (num>=(int)sizeof(size_t)) {
		if ((rnd = OPENSSL_ia32_rdrand()) == 0) return 0;

		*((size_t *)buf) = rnd;
		buf += sizeof(size_t);
		num -= sizeof(size_t);
	}
	if (num) {
		if ((rnd = OPENSSL_ia32_rdrand()) == 0) return 0;

		memcpy (buf,&rnd,num);
	}

	return 1;
	}

static int random_status (void)
{	return 1;	}

static RAND_METHOD rdrand_meth =
	{
	NULL,	/* seed */
	get_random_bytes,
	NULL,	/* cleanup */
	NULL,	/* add */
	get_random_bytes,
	random_status,
	};

static int rdrand_init(ENGINE *e)
{	return 1;	}

static const char *engine_e_rdrand_id = "rdrand";
static const char *engine_e_rdrand_name = "Intel RDRAND engine";

static int bind_helper(ENGINE *e)
	{
	if (!ENGINE_set_id(e, engine_e_rdrand_id) ||
	    !ENGINE_set_name(e, engine_e_rdrand_name) ||
	    !ENGINE_set_init_function(e, rdrand_init) ||
	    !ENGINE_set_RAND(e, &rdrand_meth) )
		return 0;

	return 1;
	}

static ENGINE *ENGINE_rdrand(void)
	{
	ENGINE *ret = ENGINE_new();
	if(!ret)
		return NULL;
	if(!bind_helper(ret))
		{
		ENGINE_free(ret);
		return NULL;
		}
	return ret;
	}

void ENGINE_load_rdrand (void)
	{
	extern unsigned int OPENSSL_ia32cap_P[];

	if (OPENSSL_ia32cap_P[1] & (1<<(62-32)))
		{
		ENGINE *toadd = ENGINE_rdrand();
		if(!toadd) return;
		ENGINE_add(toadd);
		ENGINE_free(toadd);
		ERR_clear_error();
		}
	}
#else
void ENGINE_load_rdrand (void) {}
#endif