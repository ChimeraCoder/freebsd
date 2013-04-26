
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/errno.h>
#include <i386/ibcs2/ibcs2_errno.h>

int bsd_to_ibcs2_errno[ELAST + 1] = {
	0,			/* 0 */
	IBCS2_EPERM,		/* 1 */
	IBCS2_ENOENT,		/* 2 */
	IBCS2_ESRCH,		/* 3 */
	IBCS2_EINTR,		/* 4 */
	IBCS2_EIO,		/* 5 */
	IBCS2_ENXIO,		/* 6 */
	IBCS2_E2BIG,		/* 7 */
	IBCS2_ENOEXEC,		/* 8 */
	IBCS2_EBADF,		/* 9 */
	IBCS2_ECHILD,		/* 10 */
	IBCS2_EDEADLK,		/* 11 */
	IBCS2_ENOMEM,		/* 12 */
	IBCS2_EACCES,		/* 13 */
	IBCS2_EFAULT,		/* 14 */
	IBCS2_ENOTBLK,		/* 15 */
	IBCS2_EBUSY,		/* 16 */
	IBCS2_EEXIST,		/* 17 */
	IBCS2_EXDEV,		/* 18 */
	IBCS2_ENODEV,		/* 19 */
	IBCS2_ENOTDIR,		/* 20 */
	IBCS2_EISDIR,		/* 21 */
	IBCS2_EINVAL,		/* 22 */
	IBCS2_ENFILE,		/* 23 */
	IBCS2_EMFILE,		/* 24 */
	IBCS2_ENOTTY,		/* 25 */
	IBCS2_ETXTBSY,		/* 26 */
	IBCS2_EFBIG,		/* 27 */
	IBCS2_ENOSPC,		/* 28 */
	IBCS2_ESPIPE,		/* 29 */
	IBCS2_EROFS,		/* 30 */
	IBCS2_EMLINK,		/* 31 */
	IBCS2_EPIPE,		/* 32 */
	IBCS2_EDOM,		/* 33 */
	IBCS2_ERANGE,		/* 34 */
	IBCS2_EAGAIN,		/* 35 */
	IBCS2_EINPROGRESS,	/* 36 */
	IBCS2_EALREADY,		/* 37 */
	IBCS2_ENOTSOCK,		/* 38 */
	IBCS2_EDESTADDRREQ,	/* 39 */
	IBCS2_EMSGSIZE,		/* 40 */
	IBCS2_EPROTOTYPE,	/* 41 */
	IBCS2_ENOPROTOOPT,	/* 42 */
	IBCS2_EPROTONOSUPPORT,	/* 43 */
	IBCS2_ESOCKTNOSUPPORT,	/* 44 */
	IBCS2_EOPNOTSUPP,	/* 45 */
	IBCS2_EPFNOSUPPORT,	/* 46 */
	IBCS2_EAFNOSUPPORT,	/* 47 */
	IBCS2_EADDRINUSE,	/* 48 */
	IBCS2_EADDRNOTAVAIL,	/* 49 */
	IBCS2_ENETDOWN,		/* 50 */
	IBCS2_ENETUNREACH,	/* 51 */
	IBCS2_ENETRESET,	/* 52 */
	IBCS2_ECONNABORTED,	/* 53 */
	IBCS2_ECONNRESET,	/* 54 */
	IBCS2_ENOBUFS,		/* 55 */
	IBCS2_EISCONN,		/* 56 */
	IBCS2_ENOTCONN,		/* 57 */
	IBCS2_ESHUTDOWN,	/* 58 */
	IBCS2_ETOOMANYREFS,	/* 59 */
	IBCS2_ETIMEDOUT,	/* 60 */
	IBCS2_ECONNREFUSED,	/* 61 */
	IBCS2_ELOOP,		/* 62 */
	IBCS2_ENAMETOOLONG,	/* 63 */
	IBCS2_EHOSTDOWN,	/* 64 */
	IBCS2_EHOSTUNREACH,	/* 65 */
	IBCS2_ENOTEMPTY,	/* 66 */
	0,			/* 67 */
	0,			/* 68 */
	0,			/* 69 */
	IBCS2_ESTALE,		/* 70 */
	IBCS2_EREMOTE,		/* 71 */
	0,			/* 72 */
	0,			/* 73 */
	0,			/* 74 */
	0,			/* 75 */
	0,			/* 76 */
	IBCS2_ENOLCK,		/* 77 */
	IBCS2_ENOSYS,		/* 78 */
	0,			/* 79 */
	0,			/* 80 */
	0,			/* 81 */
	IBCS2_EIDRM,		/* 82 */
	IBCS2_ENOMSG,		/* 83 */
	IBCS2_EOVERFLOW,	/* 84 */
	0,			/* 85 */
	IBCS2_EILSEQ,		/* 86 */
};