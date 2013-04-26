
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

#include "krb5_locl.h"

#define ENTROPY_NEEDED 128

static HEIMDAL_MUTEX crypto_mutex = HEIMDAL_MUTEX_INITIALIZER;

static int
seed_something(void)
{
    char buf[1024], seedfile[256];

    /* If there is a seed file, load it. But such a file cannot be trusted,
       so use 0 for the entropy estimate */
    if (RAND_file_name(seedfile, sizeof(seedfile))) {
	int fd;
	fd = open(seedfile, O_RDONLY | O_BINARY | O_CLOEXEC);
	if (fd >= 0) {
	    ssize_t ret;
	    rk_cloexec(fd);
	    ret = read(fd, buf, sizeof(buf));
	    if (ret > 0)
		RAND_add(buf, ret, 0.0);
	    close(fd);
	} else
	    seedfile[0] = '\0';
    } else
	seedfile[0] = '\0';

    /* Calling RAND_status() will try to use /dev/urandom if it exists so
       we do not have to deal with it. */
    if (RAND_status() != 1) {
#ifndef _WIN32
	krb5_context context;
	const char *p;

	/* Try using egd */
	if (!krb5_init_context(&context)) {
	    p = krb5_config_get_string(context, NULL, "libdefaults",
				       "egd_socket", NULL);
	    if (p != NULL)
		RAND_egd_bytes(p, ENTROPY_NEEDED);
	    krb5_free_context(context);
	}
#else
	/* TODO: Once a Windows CryptoAPI RAND method is defined, we
	   can use that and failover to another method. */
#endif
    }

    if (RAND_status() == 1)	{
	/* Update the seed file */
	if (seedfile[0])
	    RAND_write_file(seedfile);

	return 0;
    } else
	return -1;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_generate_random_block(void *buf, size_t len)
{
    static int rng_initialized = 0;

    HEIMDAL_MUTEX_lock(&crypto_mutex);
    if (!rng_initialized) {
	if (seed_something())
	    krb5_abortx(NULL, "Fatal: could not seed the "
			"random number generator");

	rng_initialized = 1;
    }
    HEIMDAL_MUTEX_unlock(&crypto_mutex);
    if (RAND_bytes(buf, len) <= 0)
	krb5_abortx(NULL, "Failed to generate random block");
}