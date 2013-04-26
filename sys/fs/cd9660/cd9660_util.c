
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/iconv.h>

#include <fs/cd9660/iso.h>
#include <fs/cd9660/cd9660_mount.h>

extern struct iconv_functions *cd9660_iconv;

/*
 * Get one character out of an iso filename
 * Obey joliet_level
 * Return number of bytes consumed
 */
int
isochar(isofn, isoend, joliet_level, c, clen, flags, handle)
      u_char *isofn;
      u_char *isoend;
      int joliet_level;
      u_short *c;
      int *clen;
      int flags;
      void *handle;
{
      size_t i, j, len;
      char inbuf[3], outbuf[3], *inp, *outp;

      *c = *isofn++;
      if (clen) *clen = 1;
      if (joliet_level == 0 || isofn == isoend)
              /* (00) and (01) are one byte in Joliet, too */
              return 1;

      if (flags & ISOFSMNT_KICONV && cd9660_iconv) {
              i = j = len = 2;
              inbuf[0]=(char)*(isofn - 1);
              inbuf[1]=(char)*isofn;
              inbuf[2]='\0';
              inp = inbuf;
              outp = outbuf;
              cd9660_iconv->convchr(handle, (const char **)&inp, &i, &outp, &j);
              len -= j;
              if (clen) *clen = len;
              *c = '\0';
              while(len--)
                      *c |= (*(outp - len - 1) & 0xff) << (len << 3);
      } else {
              switch (*c) {
              default:
                      *c = '?';
                      break;
              case '\0':
                      *c = *isofn;
                      break;
              }
      }

      return 2;
}

/*
 * translate and compare a filename
 * returns (fn - isofn)
 * Note: Version number plus ';' may be omitted.
 */
int
isofncmp(fn, fnlen, isofn, isolen, joliet_level, flags, handle, lhandle)
	u_char *fn;
	int fnlen;
	u_char *isofn;
	int isolen;
	int joliet_level;
	int flags;
	void *handle;
	void *lhandle;
{
	int i, j;
	u_short c, d;
	u_char *fnend = fn + fnlen, *isoend = isofn + isolen;

	for (; fn < fnend; ) {
		d = sgetrune(fn, fnend - fn, (char const **)&fn, flags, lhandle);
		if (isofn == isoend)
			return d;
		isofn += isochar(isofn, isoend, joliet_level, &c, NULL, flags, handle);
		if (c == ';') {
			if (d != ';')
				return d;
			for (i = 0; fn < fnend; i = i * 10 + *fn++ - '0') {
				if (*fn < '0' || *fn > '9') {
					return -1;
				}
			}
			for (j = 0; isofn != isoend; j = j * 10 + c - '0')
				isofn += isochar(isofn, isoend,
						 joliet_level, &c,
						 NULL, flags, handle);
			return i - j;
		}
		if (c != d) {
			if (c >= 'A' && c <= 'Z') {
				if (c + ('a' - 'A') != d) {
					if (d >= 'a' && d <= 'z')
						return d - ('a' - 'A') - c;
					else
						return d - c;
				}
			} else
				return d - c;
		}
	}
	if (isofn != isoend) {
		isofn += isochar(isofn, isoend, joliet_level, &c, NULL, flags, handle);
		switch (c) {
		default:
			return -c;
		case '.':
			if (isofn != isoend) {
				isochar(isofn, isoend, joliet_level, &c,
					NULL, flags, handle);
				if (c == ';')
					return 0;
			}
			return -1;
		case ';':
			return 0;
		}
	}
	return 0;
}

/*
 * translate a filename of length > 0
 */
void
isofntrans(infn, infnlen, outfn, outfnlen, original, assoc, joliet_level, flags, handle)
	u_char *infn;
	int infnlen;
	u_char *outfn;
	u_short *outfnlen;
	int original;
	int assoc;
	int joliet_level;
	int flags;
	void *handle;
{
	u_short c, d = '\0';
	u_char *outp = outfn, *infnend = infn + infnlen;
	int clen;

	if (assoc) {
		*outp++ = ASSOCCHAR;
	}
	for (; infn != infnend; ) {
		infn += isochar(infn, infnend, joliet_level, &c, &clen, flags, handle);

		if (!original && !joliet_level && c >= 'A' && c <= 'Z')
			c += ('a' - 'A');
		else if (!original && c == ';') {
			outp -= (d == '.');
			break;
		}
		d = c;
		while(clen--)
			*outp++ = c >> (clen << 3);
	}
	*outfnlen = outp - outfn;
}

/*
 * same as sgetrune(3)
 */
u_short
sgetrune(string, n, result, flags, handle)
	const char *string;
	size_t n;
	char const **result;
	int flags;
	void *handle;
{
	size_t i, j, len;
	char outbuf[3], *outp;
	u_short c = '\0';

	len = i = (n < 2) ? n : 2;
	j = 2;
	outp = outbuf;

	if (flags & ISOFSMNT_KICONV && cd9660_iconv) {
		cd9660_iconv->convchr(handle, (const char **)&string,
			&i, &outp, &j);
		len -= i;
	} else {
		len = 1;
		string++;
	}

	if (result) *result = string;
	while(len--) c |= (*(string - len - 1) & 0xff) << (len << 3);
	return (c);
}