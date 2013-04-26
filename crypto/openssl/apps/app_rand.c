
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

#define NON_MAIN
#include "apps.h"
#undef NON_MAIN
#include <openssl/bio.h>
#include <openssl/rand.h>


static int seeded = 0;
static int egdsocket = 0;

int app_RAND_load_file(const char *file, BIO *bio_e, int dont_warn)
	{
	int consider_randfile = (file == NULL);
	char buffer[200];
	
#ifdef OPENSSL_SYS_WINDOWS
	BIO_printf(bio_e,"Loading 'screen' into random state -");
	BIO_flush(bio_e);
	RAND_screen();
	BIO_printf(bio_e," done\n");
#endif

	if (file == NULL)
		file = RAND_file_name(buffer, sizeof buffer);
	else if (RAND_egd(file) > 0)
		{
		/* we try if the given filename is an EGD socket.
		   if it is, we don't write anything back to the file. */
		egdsocket = 1;
		return 1;
		}
	if (file == NULL || !RAND_load_file(file, -1))
		{
		if (RAND_status() == 0)
			{
			if (!dont_warn)
				{
				BIO_printf(bio_e,"unable to load 'random state'\n");
				BIO_printf(bio_e,"This means that the random number generator has not been seeded\n");
				BIO_printf(bio_e,"with much random data.\n");
				if (consider_randfile) /* explanation does not apply when a file is explicitly named */
					{
					BIO_printf(bio_e,"Consider setting the RANDFILE environment variable to point at a file that\n");
					BIO_printf(bio_e,"'random' data can be kept in (the file will be overwritten).\n");
					}
				}
			return 0;
			}
		}
	seeded = 1;
	return 1;
	}

long app_RAND_load_files(char *name)
	{
	char *p,*n;
	int last;
	long tot=0;
	int egd;
	
	for (;;)
		{
		last=0;
		for (p=name; ((*p != '\0') && (*p != LIST_SEPARATOR_CHAR)); p++);
		if (*p == '\0') last=1;
		*p='\0';
		n=name;
		name=p+1;
		if (*n == '\0') break;

		egd=RAND_egd(n);
		if (egd > 0)
			tot+=egd;
		else
			tot+=RAND_load_file(n,-1);
		if (last) break;
		}
	if (tot > 512)
		app_RAND_allow_write_file();
	return(tot);
	}

int app_RAND_write_file(const char *file, BIO *bio_e)
	{
	char buffer[200];
	
	if (egdsocket || !seeded)
		/* If we did not manage to read the seed file,
		 * we should not write a low-entropy seed file back --
		 * it would suppress a crucial warning the next time
		 * we want to use it. */
		return 0;

	if (file == NULL)
		file = RAND_file_name(buffer, sizeof buffer);
	if (file == NULL || !RAND_write_file(file))
		{
		BIO_printf(bio_e,"unable to write 'random state'\n");
		return 0;
		}
	return 1;
	}

void app_RAND_allow_write_file(void)
	{
	seeded = 1;
	}