
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

static int LZ4_uncompress_unknownOutputSize(const char *source, char *dest,
					    int isize, int maxOutputSize);

/* ARGSUSED */
static int
lz4_decompress(void *s_start, void *d_start, size_t s_len, size_t d_len, int dummy __unused)
{
	const uint8_t *src = s_start;
	uint32_t bufsiz = htonl(*(uint32_t *)src);

	/* invalid compressed buffer size encoded at start */
	if (bufsiz + 4 > s_len)
		return (1);

	/*
	 * Returns 0 on success (decompression function returned non-negative)
	 * and non-zero on failure (decompression function returned negative).
	 */
	return (LZ4_uncompress_unknownOutputSize(s_start + 4, d_start, bufsiz,
	    d_len) < 0);
}

/*
 * CPU Feature Detection
 */

/* 32 or 64 bits ? */
#if (defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || \
	defined(__amd64) || defined(__ppc64__) || defined(_WIN64) || \
	defined(__LP64__) || defined(_LP64))
#define	LZ4_ARCH64	1
#else
#define	LZ4_ARCH64	0
#endif

/*
 * Little Endian or Big Endian?
 * Note: overwrite the below #define if you know your architecture endianess.
 */
#if BYTE_ORDER == BIG_ENDIAN
#define	LZ4_BIG_ENDIAN	1
#else
	/*
	 * Little Endian assumed. PDP Endian and other very rare endian format
	 * are unsupported.
	 */
#endif

/*
 * Compiler Options
 */
#if __STDC_VERSION__ >= 199901L	/* C99 */
/* "restrict" is a known keyword */
#else
/* Disable restrict */
#define	restrict
#endif

#define	GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)

#define	lz4_bswap16(x) ((unsigned short int) ((((x) >> 8) & 0xffu) \
	| (((x) & 0xffu) << 8)))

#if (GCC_VERSION >= 302) || (__INTEL_COMPILER >= 800) || defined(__clang__)
#define	expect(expr, value)    (__builtin_expect((expr), (value)))
#else
#define	expect(expr, value)    (expr)
#endif

#define	likely(expr)	expect((expr) != 0, 1)
#define	unlikely(expr)	expect((expr) != 0, 0)

/* Basic types */
#define	BYTE	uint8_t
#define	U16	uint16_t
#define	U32	uint32_t
#define	S32	int32_t
#define	U64	uint64_t

typedef struct _U16_S {
	U16 v;
} U16_S;
typedef struct _U32_S {
	U32 v;
} U32_S;
typedef struct _U64_S {
	U64 v;
} U64_S;

#define	A64(x)	(((U64_S *)(x))->v)
#define	A32(x)	(((U32_S *)(x))->v)
#define	A16(x)	(((U16_S *)(x))->v)

/*
 * Constants
 */
#define	MINMATCH 4

#define	COPYLENGTH 8
#define	LASTLITERALS 5

#define	ML_BITS 4
#define	ML_MASK ((1U<<ML_BITS)-1)
#define	RUN_BITS (8-ML_BITS)
#define	RUN_MASK ((1U<<RUN_BITS)-1)

/*
 * Architecture-specific macros
 */
#if LZ4_ARCH64
#define	STEPSIZE 8
#define	UARCH U64
#define	AARCH A64
#define	LZ4_COPYSTEP(s, d)	A64(d) = A64(s); d += 8; s += 8;
#define	LZ4_COPYPACKET(s, d)	LZ4_COPYSTEP(s, d)
#define	LZ4_SECURECOPY(s, d, e)	if (d < e) LZ4_WILDCOPY(s, d, e)
#define	HTYPE U32
#define	INITBASE(base)		const BYTE* const base = ip
#else
#define	STEPSIZE 4
#define	UARCH U32
#define	AARCH A32
#define	LZ4_COPYSTEP(s, d)	A32(d) = A32(s); d += 4; s += 4;
#define	LZ4_COPYPACKET(s, d)	LZ4_COPYSTEP(s, d); LZ4_COPYSTEP(s, d);
#define	LZ4_SECURECOPY		LZ4_WILDCOPY
#define	HTYPE const BYTE*
#define	INITBASE(base)		const int base = 0
#endif

#if (defined(LZ4_BIG_ENDIAN) && !defined(BIG_ENDIAN_NATIVE_BUT_INCOMPATIBLE))
#define	LZ4_READ_LITTLEENDIAN_16(d, s, p) \
	{ U16 v = A16(p); v = lz4_bswap16(v); d = (s) - v; }
#define	LZ4_WRITE_LITTLEENDIAN_16(p, i) \
	{ U16 v = (U16)(i); v = lz4_bswap16(v); A16(p) = v; p += 2; }
#else
#define	LZ4_READ_LITTLEENDIAN_16(d, s, p) { d = (s) - A16(p); }
#define	LZ4_WRITE_LITTLEENDIAN_16(p, v)  { A16(p) = v; p += 2; }
#endif

/* Macros */
#define	LZ4_WILDCOPY(s, d, e) do { LZ4_COPYPACKET(s, d) } while (d < e);

/* Decompression functions */

static int
LZ4_uncompress_unknownOutputSize(const char *source,
    char *dest, int isize, int maxOutputSize)
{
	/* Local Variables */
	const BYTE *restrict ip = (const BYTE *) source;
	const BYTE *const iend = ip + isize;
	const BYTE *restrict ref;

	BYTE *restrict op = (BYTE *) dest;
	BYTE *const oend = op + maxOutputSize;
	BYTE *cpy;

	size_t dec[] = { 0, 3, 2, 3, 0, 0, 0, 0 };

	/* Main Loop */
	while (ip < iend) {
		BYTE token;
		int length;

		/* get runlength */
		token = *ip++;
		if ((length = (token >> ML_BITS)) == RUN_MASK) {
			int s = 255;
			while ((ip < iend) && (s == 255)) {
				s = *ip++;
				length += s;
			}
		}
		/* copy literals */
		cpy = op + length;
		if ((cpy > oend - COPYLENGTH) ||
		    (ip + length > iend - COPYLENGTH)) {
			if (cpy > oend)
				/*
				 * Error: request to write beyond destination
				 * buffer.
				 */
				goto _output_error;
			if (ip + length > iend)
				/*
				 * Error : request to read beyond source
				 * buffer.
				 */
				goto _output_error;
			memcpy(op, ip, length);
			op += length;
			ip += length;
			if (ip < iend)
				/* Error : LZ4 format violation */
				goto _output_error;
			/* Necessarily EOF, due to parsing restrictions. */
			break;
		}
		LZ4_WILDCOPY(ip, op, cpy);
		ip -= (op - cpy);
		op = cpy;

		/* get offset */
		LZ4_READ_LITTLEENDIAN_16(ref, cpy, ip);
		ip += 2;
		if (ref < (BYTE * const) dest)
			/*
			 * Error: offset creates reference outside of
			 * destination buffer.
			 */
			goto _output_error;

		/* get matchlength */
		if ((length = (token & ML_MASK)) == ML_MASK) {
			while (ip < iend) {
				int s = *ip++;
				length += s;
				if (s == 255)
					continue;
				break;
			}
		}
		/* copy repeated sequence */
		if unlikely(op - ref < STEPSIZE) {
#if LZ4_ARCH64
			size_t dec2table[] = { 0, 0, 0, -1, 0, 1, 2, 3 };
			size_t dec2 = dec2table[op - ref];
#else
			const int dec2 = 0;
#endif
			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			ref -= dec[op - ref];
			A32(op) = A32(ref);
			op += STEPSIZE - 4;
			ref -= dec2;
		} else {
			LZ4_COPYSTEP(ref, op);
		}
		cpy = op + length - (STEPSIZE - 4);
		if (cpy > oend - COPYLENGTH) {
			if (cpy > oend)
				/*
				 * Error: request to write outside of
				 * destination buffer.
				 */
				goto _output_error;
			LZ4_SECURECOPY(ref, op, (oend - COPYLENGTH));
			while (op < cpy)
				*op++ = *ref++;
			op = cpy;
			if (op == oend)
				/*
				 * Check EOF (should never happen, since last
				 * 5 bytes are supposed to be literals).
				 */
				break;
			continue;
		}
		LZ4_SECURECOPY(ref, op, cpy);
		op = cpy;	/* correction */
	}

	/* end of decoding */
	return (int)(((char *)op) - dest);

	/* write overflow error detected */
	_output_error:
	return (int)(-(((char *)ip) - source));
}