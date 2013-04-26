
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <ctf_impl.h>
#include <sys/kobj.h>
#include <sys/kobj_impl.h>

/*
 * This module is used both during the normal operation of the kernel (i.e.
 * after kmem has been initialized) and during boot (before unix`_start has
 * been called).  kobj_alloc is able to tell the difference between the two
 * cases, and as such must be used instead of kmem_alloc.
 */

void *
ctf_data_alloc(size_t size)
{
	void *buf = kobj_alloc(size, KM_NOWAIT|KM_SCRATCH);

	if (buf == NULL)
		return (MAP_FAILED);

	return (buf);
}

void
ctf_data_free(void *buf, size_t size)
{
	kobj_free(buf, size);
}

/*ARGSUSED*/
void
ctf_data_protect(void *buf, size_t size)
{
	/* we don't support this operation in the kernel */
}

void *
ctf_alloc(size_t size)
{
	return (kobj_alloc(size, KM_NOWAIT|KM_TMP));
}

/*ARGSUSED*/
void
ctf_free(void *buf, size_t size)
{
	kobj_free(buf, size);
}

/*ARGSUSED*/
const char *
ctf_strerror(int err)
{
	return (NULL); /* we don't support this operation in the kernel */
}

/*PRINTFLIKE1*/
void
ctf_dprintf(const char *format, ...)
{
	if (_libctf_debug) {
		va_list alist;

		va_start(alist, format);
		(void) printf("ctf DEBUG: ");
		(void) vprintf(format, alist);
		va_end(alist);
	}
}