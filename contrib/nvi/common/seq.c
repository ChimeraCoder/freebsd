
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

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)seq.c	10.10 (Berkeley) 3/30/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>

#include <bitstring.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

/*
 * seq_set --
 *	Internal version to enter a sequence.
 *
 * PUBLIC: int seq_set __P((SCR *, CHAR_T *,
 * PUBLIC:    size_t, CHAR_T *, size_t, CHAR_T *, size_t, seq_t, int));
 */
int
seq_set(sp, name, nlen, input, ilen, output, olen, stype, flags)
	SCR *sp;
	CHAR_T *name, *input, *output;
	size_t nlen, ilen, olen;
	seq_t stype;
	int flags;
{
	CHAR_T *p;
	SEQ *lastqp, *qp;
	int sv_errno;

	/*
	 * An input string must always be present.  The output string
	 * can be NULL, when set internally, that's how we throw away
	 * input.
	 *
	 * Just replace the output field if the string already set.
	 */
	if ((qp =
	    seq_find(sp, &lastqp, NULL, input, ilen, stype, NULL)) != NULL) {
		if (LF_ISSET(SEQ_NOOVERWRITE))
			return (0);
		if (output == NULL || olen == 0) {
			p = NULL;
			olen = 0;
		} else if ((p = v_strdup(sp, output, olen)) == NULL) {
			sv_errno = errno;
			goto mem1;
		}
		if (qp->output != NULL)
			free(qp->output);
		qp->olen = olen;
		qp->output = p;
		return (0);
	}

	/* Allocate and initialize SEQ structure. */
	CALLOC(sp, qp, SEQ *, 1, sizeof(SEQ));
	if (qp == NULL) {
		sv_errno = errno;
		goto mem1;
	}

	/* Name. */
	if (name == NULL || nlen == 0)
		qp->name = NULL;
	else if ((qp->name = v_strdup(sp, name, nlen)) == NULL) {
		sv_errno = errno;
		goto mem2;
	}
	qp->nlen = nlen;

	/* Input. */
	if ((qp->input = v_strdup(sp, input, ilen)) == NULL) {
		sv_errno = errno;
		goto mem3;
	}
	qp->ilen = ilen;

	/* Output. */
	if (output == NULL) {
		qp->output = NULL;
		olen = 0;
	} else if ((qp->output = v_strdup(sp, output, olen)) == NULL) {
		sv_errno = errno;
		free(qp->input);
mem3:		if (qp->name != NULL)
			free(qp->name);
mem2:		free(qp);
mem1:		errno = sv_errno;
		msgq(sp, M_SYSERR, NULL);
		return (1);
	}
	qp->olen = olen;

	/* Type, flags. */
	qp->stype = stype;
	qp->flags = flags;

	/* Link into the chain. */
	if (lastqp == NULL) {
		LIST_INSERT_HEAD(&sp->gp->seqq, qp, q);
	} else {
		LIST_INSERT_AFTER(lastqp, qp, q);
	}

	/* Set the fast lookup bit. */
	if (qp->input[0] < MAX_BIT_SEQ)
		bit_set(sp->gp->seqb, qp->input[0]);

	return (0);
}

/*
 * seq_delete --
 *	Delete a sequence.
 *
 * PUBLIC: int seq_delete __P((SCR *, CHAR_T *, size_t, seq_t));
 */
int
seq_delete(sp, input, ilen, stype)
	SCR *sp;
	CHAR_T *input;
	size_t ilen;
	seq_t stype;
{
	SEQ *qp;

	if ((qp = seq_find(sp, NULL, NULL, input, ilen, stype, NULL)) == NULL)
		return (1);
	return (seq_mdel(qp));
}

/*
 * seq_mdel --
 *	Delete a map entry, without lookup.
 *
 * PUBLIC: int seq_mdel __P((SEQ *));
 */
int
seq_mdel(qp)
	SEQ *qp;
{
	LIST_REMOVE(qp, q);
	if (qp->name != NULL)
		free(qp->name);
	free(qp->input);
	if (qp->output != NULL)
		free(qp->output);
	free(qp);
	return (0);
}

/*
 * seq_find --
 *	Search the sequence list for a match to a buffer, if ispartial
 *	isn't NULL, partial matches count.
 *
 * PUBLIC: SEQ *seq_find
 * PUBLIC:    __P((SCR *, SEQ **, EVENT *, CHAR_T *, size_t, seq_t, int *));
 */
SEQ *
seq_find(sp, lastqp, e_input, c_input, ilen, stype, ispartialp)
	SCR *sp;
	SEQ **lastqp;
	EVENT *e_input;
	CHAR_T *c_input;
	size_t ilen;
	seq_t stype;
	int *ispartialp;
{
	SEQ *lqp, *qp;
	int diff;

	/*
	 * Ispartialp is a location where we return if there was a
	 * partial match, i.e. if the string were extended it might
	 * match something.
	 *
	 * XXX
	 * Overload the meaning of ispartialp; only the terminal key
	 * search doesn't want the search limited to complete matches,
	 * i.e. ilen may be longer than the match.
	 */
	if (ispartialp != NULL)
		*ispartialp = 0;
	for (lqp = NULL, qp = sp->gp->seqq.lh_first;
	    qp != NULL; lqp = qp, qp = qp->q.le_next) {
		/*
		 * Fast checks on the first character and type, and then
		 * a real comparison.
		 */
		if (e_input == NULL) {
			if (qp->input[0] > c_input[0])
				break;
			if (qp->input[0] < c_input[0] ||
			    qp->stype != stype || F_ISSET(qp, SEQ_FUNCMAP))
				continue;
			diff = memcmp(qp->input, c_input, MIN(qp->ilen, ilen));
		} else {
			if (qp->input[0] > e_input->e_c)
				break;
			if (qp->input[0] < e_input->e_c ||
			    qp->stype != stype || F_ISSET(qp, SEQ_FUNCMAP))
				continue;
			diff =
			    e_memcmp(qp->input, e_input, MIN(qp->ilen, ilen));
		}
		if (diff > 0)
			break;
		if (diff < 0)
			continue;
		/*
		 * If the entry is the same length as the string, return a
		 * match.  If the entry is shorter than the string, return a
		 * match if called from the terminal key routine.  Otherwise,
		 * keep searching for a complete match.
		 */
		if (qp->ilen <= ilen) {
			if (qp->ilen == ilen || ispartialp != NULL) {
				if (lastqp != NULL)
					*lastqp = lqp;
				return (qp);
			}
			continue;
		}
		/*
		 * If the entry longer than the string, return partial match
		 * if called from the terminal key routine.  Otherwise, no
		 * match.
		 */
		if (ispartialp != NULL)
			*ispartialp = 1;
		break;
	}
	if (lastqp != NULL)
		*lastqp = lqp;
	return (NULL);
}

/*
 * seq_close --
 *	Discard all sequences.
 *
 * PUBLIC: void seq_close __P((GS *));
 */
void
seq_close(gp)
	GS *gp;
{
	SEQ *qp;

	while ((qp = gp->seqq.lh_first) != NULL) {
		if (qp->name != NULL)
			free(qp->name);
		if (qp->input != NULL)
			free(qp->input);
		if (qp->output != NULL)
			free(qp->output);
		LIST_REMOVE(qp, q);
		free(qp);
	}
}

/*
 * seq_dump --
 *	Display the sequence entries of a specified type.
 *
 * PUBLIC: int seq_dump __P((SCR *, seq_t, int));
 */
int
seq_dump(sp, stype, isname)
	SCR *sp;
	seq_t stype;
	int isname;
{
	CHAR_T *p;
	GS *gp;
	SEQ *qp;
	int cnt, len, olen;

	cnt = 0;
	gp = sp->gp;
	for (qp = gp->seqq.lh_first; qp != NULL; qp = qp->q.le_next) {
		if (stype != qp->stype || F_ISSET(qp, SEQ_FUNCMAP))
			continue;
		++cnt;
		for (p = qp->input,
		    olen = qp->ilen, len = 0; olen > 0; --olen, ++p)
			len += ex_puts(sp, KEY_NAME(sp, *p));
		for (len = STANDARD_TAB - len % STANDARD_TAB; len > 0;)
			len -= ex_puts(sp, " ");

		if (qp->output != NULL)
			for (p = qp->output,
			    olen = qp->olen, len = 0; olen > 0; --olen, ++p)
				len += ex_puts(sp, KEY_NAME(sp, *p));
		else
			len = 0;

		if (isname && qp->name != NULL) {
			for (len = STANDARD_TAB - len % STANDARD_TAB; len > 0;)
				len -= ex_puts(sp, " ");
			for (p = qp->name,
			    olen = qp->nlen; olen > 0; --olen, ++p)
				(void)ex_puts(sp, KEY_NAME(sp, *p));
		}
		(void)ex_puts(sp, "\n");
	}
	return (cnt);
}

/*
 * seq_save --
 *	Save the sequence entries to a file.
 *
 * PUBLIC: int seq_save __P((SCR *, FILE *, char *, seq_t));
 */
int
seq_save(sp, fp, prefix, stype)
	SCR *sp;
	FILE *fp;
	char *prefix;
	seq_t stype;
{
	CHAR_T *p;
	SEQ *qp;
	size_t olen;
	int ch;

	/* Write a sequence command for all keys the user defined. */
	for (qp = sp->gp->seqq.lh_first; qp != NULL; qp = qp->q.le_next) {
		if (stype != qp->stype || !F_ISSET(qp, SEQ_USERDEF))
			continue;
		if (prefix)
			(void)fprintf(fp, "%s", prefix);
		for (p = qp->input, olen = qp->ilen; olen > 0; --olen) {
			ch = *p++;
			if (ch == CH_LITERAL || ch == '|' ||
			    isblank(ch) || KEY_VAL(sp, ch) == K_NL)
				(void)putc(CH_LITERAL, fp);
			(void)putc(ch, fp);
		}
		(void)putc(' ', fp);
		if (qp->output != NULL)
			for (p = qp->output,
			    olen = qp->olen; olen > 0; --olen) {
				ch = *p++;
				if (ch == CH_LITERAL || ch == '|' ||
				    KEY_VAL(sp, ch) == K_NL)
					(void)putc(CH_LITERAL, fp);
				(void)putc(ch, fp);
			}
		(void)putc('\n', fp);
	}
	return (0);
}

/*
 * e_memcmp --
 *	Compare a string of EVENT's to a string of CHAR_T's.
 *
 * PUBLIC: int e_memcmp __P((CHAR_T *, EVENT *, size_t));
 */
int
e_memcmp(p1, ep, n)
	CHAR_T *p1;
	EVENT *ep;
	size_t n;
{
	if (n != 0) {
                do {
                        if (*p1++ != ep->e_c)
                                return (*--p1 - ep->e_c);
			++ep;
                } while (--n != 0);
        }
        return (0);
}