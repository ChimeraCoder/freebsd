
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

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/buffer.h>

char *BUF_strdup(const char *str)
	{
	if (str == NULL) return(NULL);
	return BUF_strndup(str, strlen(str));
	}

char *BUF_strndup(const char *str, size_t siz)
	{
	char *ret;

	if (str == NULL) return(NULL);

	ret=OPENSSL_malloc(siz+1);
	if (ret == NULL) 
		{
		BUFerr(BUF_F_BUF_STRNDUP,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	BUF_strlcpy(ret,str,siz+1);
	return(ret);
	}

void *BUF_memdup(const void *data, size_t siz)
	{
	void *ret;

	if (data == NULL) return(NULL);

	ret=OPENSSL_malloc(siz);
	if (ret == NULL) 
		{
		BUFerr(BUF_F_BUF_MEMDUP,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	return memcpy(ret, data, siz);
	}	

size_t BUF_strlcpy(char *dst, const char *src, size_t size)
	{
	size_t l = 0;
	for(; size > 1 && *src; size--)
		{
		*dst++ = *src++;
		l++;
		}
	if (size)
		*dst = '\0';
	return l + strlen(src);
	}

size_t BUF_strlcat(char *dst, const char *src, size_t size)
	{
	size_t l = 0;
	for(; size > 0 && *dst; size--, dst++)
		l++;
	return l + BUF_strlcpy(dst, src, size);
	}