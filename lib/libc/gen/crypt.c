
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

#if defined(LIBC_SCCS) && !defined(lint)
/* from static char sccsid[] = "@(#)crypt.c	5.11 (Berkeley) 6/25/91"; */
#endif /* LIBC_SCCS and not lint */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * UNIX password, and DES, encryption.
 *
 * since this is non-exportable, this is just a dummy.  if you want real
 * encryption, make sure you've got libcrypt.a around.
 */

__warn_references(des_setkey,
	"WARNING!  des_setkey(3) not present in the system!");

/* ARGSUSED */
int
des_setkey(const char *key __unused)
{
	fprintf(stderr, "WARNING!  des_setkey(3) not present in the system!\n");
	return (0);
}

__warn_references(des_cipher,
	"WARNING!  des_cipher(3) not present in the system!");

/* ARGSUSED */
int
des_cipher(const char *in, char *out, long salt __unused, int num_iter __unused)
{
	fprintf(stderr, "WARNING!  des_cipher(3) not present in the system!\n");
	bcopy(in, out, 8);
	return (0);
}

__warn_references(setkey,
	"WARNING!  setkey(3) not present in the system!");

/* ARGSUSED */
int
setkey(const char *key __unused)
{
	fprintf(stderr, "WARNING!  setkey(3) not present in the system!\n");
	return (0);
}

__warn_references(encrypt,
	"WARNING!  encrypt(3) not present in the system!");

/* ARGSUSED */
int
encrypt(char *block __unused, int flag __unused)
{
	fprintf(stderr, "WARNING!  encrypt(3) not present in the system!\n");
	return (0);
}