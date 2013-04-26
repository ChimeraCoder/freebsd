
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

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

#ifdef __i386__
#include <ieeefp.h>
#endif

#include "../stdio/floatio.h"
#include "fpmath.h"
#include "gdtoaimp.h"

#if (LDBL_MANT_DIG > DBL_MANT_DIG)

/* Strings values used by dtoa() */
#define	INFSTR	"Infinity"
#define	NANSTR	"NaN"

#ifdef LDBL_IMPLICIT_NBIT
#define	MANH_SIZE	LDBL_MANH_SIZE
#else
#define	MANH_SIZE	(LDBL_MANH_SIZE - 1)
#endif

#if MANH_SIZE > 32
typedef uint64_t manh_t;
#else
typedef uint32_t manh_t;
#endif

#if LDBL_MANL_SIZE > 32
typedef uint64_t manl_t;
#else
typedef uint32_t manl_t;
#endif

#define	LDBL_ADJ	(LDBL_MAX_EXP - 2)
#define	SIGFIGS		((LDBL_MANT_DIG + 3) / 4 + 1)

static const float one[] = { 1.0f, -1.0f };

/*
 * This is the long double version of __hdtoa().
 */
char *
__hldtoa(long double e, const char *xdigs, int ndigits, int *decpt, int *sign,
    char **rve)
{
	union IEEEl2bits u;
	char *s, *s0;
	manh_t manh;
	manl_t manl;
	int bufsize;
#ifdef __i386__
	fp_prec_t oldprec;
#endif

	u.e = e;
	*sign = u.bits.sign;

	switch (fpclassify(e)) {
	case FP_NORMAL:
		*decpt = u.bits.exp - LDBL_ADJ;
		break;
	case FP_ZERO:
		*decpt = 1;
		return (nrv_alloc("0", rve, 1));
	case FP_SUBNORMAL:
#ifdef __i386__
		oldprec = fpsetprec(FP_PE);
#endif
		u.e *= 0x1p514L;
		*decpt = u.bits.exp - (514 + LDBL_ADJ);
#ifdef __i386__
		fpsetprec(oldprec);
#endif
		break;
	case FP_INFINITE:
		*decpt = INT_MAX;
		return (nrv_alloc(INFSTR, rve, sizeof(INFSTR) - 1));
	default:	/* FP_NAN or unrecognized */
		*decpt = INT_MAX;
		return (nrv_alloc(NANSTR, rve, sizeof(NANSTR) - 1));
	}

	/* FP_NORMAL or FP_SUBNORMAL */

	if (ndigits == 0)		/* dtoa() compatibility */
		ndigits = 1;

	/*
	 * If ndigits < 0, we are expected to auto-size, so we allocate
	 * enough space for all the digits.
	 */
	bufsize = (ndigits > 0) ? ndigits : SIGFIGS;
	s0 = rv_alloc(bufsize);

	/* Round to the desired number of digits. */
	if (SIGFIGS > ndigits && ndigits > 0) {
		float redux = one[u.bits.sign];
		int offset = 4 * ndigits + LDBL_MAX_EXP - 4 - LDBL_MANT_DIG;
#ifdef __i386__
		oldprec = fpsetprec(FP_PE);
#endif
		u.bits.exp = offset;
		u.e += redux;
		u.e -= redux;
		*decpt += u.bits.exp - offset;
#ifdef __i386__
		fpsetprec(oldprec);
#endif
	}

	mask_nbit_l(u);
	manh = u.bits.manh;
	manl = u.bits.manl;	
	*s0 = '1';
	for (s = s0 + 1; s < s0 + bufsize; s++) {
		*s = xdigs[(manh >> (MANH_SIZE - 4)) & 0xf];
		manh = (manh << 4) | (manl >> (LDBL_MANL_SIZE - 4));
		manl <<= 4;
	}

	/* If ndigits < 0, we are expected to auto-size the precision. */
	if (ndigits < 0) {
		for (ndigits = SIGFIGS; s0[ndigits - 1] == '0'; ndigits--)
			;
	}

	s = s0 + ndigits;
	*s = '\0';
	if (rve != NULL)
		*rve = s;
	return (s0);
}

#else	/* (LDBL_MANT_DIG == DBL_MANT_DIG) */

char *
__hldtoa(long double e, const char *xdigs, int ndigits, int *decpt, int *sign,
    char **rve)
{

	return (__hdtoa((double)e, xdigs, ndigits, decpt, sign, rve));
}

#endif	/* (LDBL_MANT_DIG == DBL_MANT_DIG) */