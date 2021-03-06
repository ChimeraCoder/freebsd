
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)conv.c	8.3 (Berkeley) 4/2/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include <err.h>
#include <inttypes.h>
#include <string.h>

#include "dd.h"
#include "extern.h"

/*
 * def --
 * Copy input to output.  Input is buffered until reaches obs, and then
 * output until less than obs remains.  Only a single buffer is used.
 * Worst case buffer calculation is (ibs + obs - 1).
 */
void
def(void)
{
	u_char *inp;
	const u_char *t;
	size_t cnt;

	if ((t = ctab) != NULL)
		for (inp = in.dbp - (cnt = in.dbrcnt); cnt--; ++inp)
			*inp = t[*inp];

	/* Make the output buffer look right. */
	out.dbp = in.dbp;
	out.dbcnt = in.dbcnt;

	if (in.dbcnt >= out.dbsz) {
		/* If the output buffer is full, write it. */
		dd_out(0);

		/*
		 * Ddout copies the leftover output to the beginning of
		 * the buffer and resets the output buffer.  Reset the
		 * input buffer to match it.
	 	 */
		in.dbp = out.dbp;
		in.dbcnt = out.dbcnt;
	}
}

void
def_close(void)
{
	/* Just update the count, everything is already in the buffer. */
	if (in.dbcnt)
		out.dbcnt = in.dbcnt;
}

/*
 * Copy variable length newline terminated records with a max size cbsz
 * bytes to output.  Records less than cbs are padded with spaces.
 *
 * max in buffer:  MAX(ibs, cbsz)
 * max out buffer: obs + cbsz
 */
void
block(void)
{
	u_char *inp, *outp;
	const u_char *t;
	size_t cnt, maxlen;
	static int intrunc;
	int ch;

	/*
	 * Record truncation can cross block boundaries.  If currently in a
	 * truncation state, keep tossing characters until reach a newline.
	 * Start at the beginning of the buffer, as the input buffer is always
	 * left empty.
	 */
	if (intrunc) {
		for (inp = in.db, cnt = in.dbrcnt; cnt && *inp++ != '\n'; --cnt)
			;
		if (!cnt) {
			in.dbcnt = 0;
			in.dbp = in.db;
			return;
		}
		intrunc = 0;
		/* Adjust the input buffer numbers. */
		in.dbcnt = cnt - 1;
		in.dbp = inp + cnt - 1;
	}

	/*
	 * Copy records (max cbsz size chunks) into the output buffer.  The
	 * translation is done as we copy into the output buffer.
	 */
	ch = 0;
	for (inp = in.dbp - in.dbcnt, outp = out.dbp; in.dbcnt;) {
		maxlen = MIN(cbsz, in.dbcnt);
		if ((t = ctab) != NULL)
			for (cnt = 0; cnt < maxlen && (ch = *inp++) != '\n';
			    ++cnt)
				*outp++ = t[ch];
		else
			for (cnt = 0; cnt < maxlen && (ch = *inp++) != '\n';
			    ++cnt)
				*outp++ = ch;
		/*
		 * Check for short record without a newline.  Reassemble the
		 * input block.
		 */
		if (ch != '\n' && in.dbcnt < cbsz) {
			(void)memmove(in.db, in.dbp - in.dbcnt, in.dbcnt);
			break;
		}

		/* Adjust the input buffer numbers. */
		in.dbcnt -= cnt;
		if (ch == '\n')
			--in.dbcnt;

		/* Pad short records with spaces. */
		if (cnt < cbsz)
			(void)memset(outp, ctab ? ctab[' '] : ' ', cbsz - cnt);
		else {
			/*
			 * If the next character wouldn't have ended the
			 * block, it's a truncation.
			 */
			if (!in.dbcnt || *inp != '\n')
				++st.trunc;

			/* Toss characters to a newline. */
			for (; in.dbcnt && *inp++ != '\n'; --in.dbcnt)
				;
			if (!in.dbcnt)
				intrunc = 1;
			else
				--in.dbcnt;
		}

		/* Adjust output buffer numbers. */
		out.dbp += cbsz;
		if ((out.dbcnt += cbsz) >= out.dbsz)
			dd_out(0);
		outp = out.dbp;
	}
	in.dbp = in.db + in.dbcnt;
}

void
block_close(void)
{
	/*
	 * Copy any remaining data into the output buffer and pad to a record.
	 * Don't worry about truncation or translation, the input buffer is
	 * always empty when truncating, and no characters have been added for
	 * translation.  The bottom line is that anything left in the input
	 * buffer is a truncated record.  Anything left in the output buffer
	 * just wasn't big enough.
	 */
	if (in.dbcnt) {
		++st.trunc;
		(void)memmove(out.dbp, in.dbp - in.dbcnt, in.dbcnt);
		(void)memset(out.dbp + in.dbcnt, ctab ? ctab[' '] : ' ',
		    cbsz - in.dbcnt);
		out.dbcnt += cbsz;
	}
}

/*
 * Convert fixed length (cbsz) records to variable length.  Deletes any
 * trailing blanks and appends a newline.
 *
 * max in buffer:  MAX(ibs, cbsz) + cbsz
 * max out buffer: obs + cbsz
 */
void
unblock(void)
{
	u_char *inp;
	const u_char *t;
	size_t cnt;

	/* Translation and case conversion. */
	if ((t = ctab) != NULL)
		for (inp = in.dbp - (cnt = in.dbrcnt); cnt--; ++inp)
			*inp = t[*inp];
	/*
	 * Copy records (max cbsz size chunks) into the output buffer.  The
	 * translation has to already be done or we might not recognize the
	 * spaces.
	 */
	for (inp = in.db; in.dbcnt >= cbsz; inp += cbsz, in.dbcnt -= cbsz) {
		for (t = inp + cbsz - 1; t >= inp && *t == ' '; --t)
			;
		if (t >= inp) {
			cnt = t - inp + 1;
			(void)memmove(out.dbp, inp, cnt);
			out.dbp += cnt;
			out.dbcnt += cnt;
		}
		*out.dbp++ = '\n';
		if (++out.dbcnt >= out.dbsz)
			dd_out(0);
	}
	if (in.dbcnt)
		(void)memmove(in.db, in.dbp - in.dbcnt, in.dbcnt);
	in.dbp = in.db + in.dbcnt;
}

void
unblock_close(void)
{
	u_char *t;
	size_t cnt;

	if (in.dbcnt) {
		warnx("%s: short input record", in.name);
		for (t = in.db + in.dbcnt - 1; t >= in.db && *t == ' '; --t)
			;
		if (t >= in.db) {
			cnt = t - in.db + 1;
			(void)memmove(out.dbp, in.db, cnt);
			out.dbp += cnt;
			out.dbcnt += cnt;
		}
		++out.dbcnt;
		*out.dbp++ = '\n';
	}
}