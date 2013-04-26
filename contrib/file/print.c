
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

#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: print.c,v 1.71 2011/09/20 15:28:09 christos Exp $")
#endif  /* lint */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <time.h>

#define SZOF(a)	(sizeof(a) / sizeof(a[0]))

#ifndef COMPILE_ONLY
protected void
file_mdump(struct magic *m)
{
	private const char optyp[] = { FILE_OPS };

	(void) fprintf(stderr, "%u: %.*s %u", m->lineno,
	    (m->cont_level & 7) + 1, ">>>>>>>>", m->offset);

	if (m->flag & INDIR) {
		(void) fprintf(stderr, "(%s,",
			       /* Note: type is unsigned */
			       (m->in_type < file_nnames) ? 
					file_names[m->in_type] : "*bad*");
		if (m->in_op & FILE_OPINVERSE)
			(void) fputc('~', stderr);
		(void) fprintf(stderr, "%c%u),",
			       ((size_t)(m->in_op & FILE_OPS_MASK) <
			       SZOF(optyp)) ? 
					optyp[m->in_op & FILE_OPS_MASK] : '?',
				m->in_offset);
	}
	(void) fprintf(stderr, " %s%s", (m->flag & UNSIGNED) ? "u" : "",
		       /* Note: type is unsigned */
		       (m->type < file_nnames) ? file_names[m->type] : "*bad*");
	if (m->mask_op & FILE_OPINVERSE)
		(void) fputc('~', stderr);

	if (IS_STRING(m->type)) {
		if (m->str_flags) {
			(void) fputc('/', stderr);
			if (m->str_flags & STRING_COMPACT_WHITESPACE) 
				(void) fputc(CHAR_COMPACT_WHITESPACE, stderr);
			if (m->str_flags & STRING_COMPACT_OPTIONAL_WHITESPACE) 
				(void) fputc(CHAR_COMPACT_OPTIONAL_WHITESPACE,
				    stderr);
			if (m->str_flags & STRING_IGNORE_LOWERCASE) 
				(void) fputc(CHAR_IGNORE_LOWERCASE, stderr);
			if (m->str_flags & STRING_IGNORE_UPPERCASE) 
				(void) fputc(CHAR_IGNORE_UPPERCASE, stderr);
			if (m->str_flags & REGEX_OFFSET_START) 
				(void) fputc(CHAR_REGEX_OFFSET_START, stderr);
			if (m->str_flags & STRING_TEXTTEST)
				(void) fputc(CHAR_TEXTTEST, stderr);
			if (m->str_flags & STRING_BINTEST)
				(void) fputc(CHAR_BINTEST, stderr);
			if (m->str_flags & PSTRING_1_BE)
				(void) fputc(CHAR_PSTRING_1_BE, stderr);
			if (m->str_flags & PSTRING_2_BE)
				(void) fputc(CHAR_PSTRING_2_BE, stderr);
			if (m->str_flags & PSTRING_2_LE)
				(void) fputc(CHAR_PSTRING_2_LE, stderr);
			if (m->str_flags & PSTRING_4_BE)
				(void) fputc(CHAR_PSTRING_4_BE, stderr);
			if (m->str_flags & PSTRING_4_LE)
				(void) fputc(CHAR_PSTRING_4_LE, stderr);
			if (m->str_flags & PSTRING_LENGTH_INCLUDES_ITSELF)
				(void) fputc(
				    CHAR_PSTRING_LENGTH_INCLUDES_ITSELF,
				    stderr);
		}
		if (m->str_range)
			(void) fprintf(stderr, "/%u", m->str_range);
	}
	else {
		if ((size_t)(m->mask_op & FILE_OPS_MASK) < SZOF(optyp))
			(void) fputc(optyp[m->mask_op & FILE_OPS_MASK], stderr);
		else
			(void) fputc('?', stderr);
			
		if (m->num_mask) {
			(void) fprintf(stderr, "%.8llx",
			    (unsigned long long)m->num_mask);
		}
	}
	(void) fprintf(stderr, ",%c", m->reln);

	if (m->reln != 'x') {
		switch (m->type) {
		case FILE_BYTE:
		case FILE_SHORT:
		case FILE_LONG:
		case FILE_LESHORT:
		case FILE_LELONG:
		case FILE_MELONG:
		case FILE_BESHORT:
		case FILE_BELONG:
			(void) fprintf(stderr, "%d", m->value.l);
			break;
		case FILE_BEQUAD:
		case FILE_LEQUAD:
		case FILE_QUAD:
			(void) fprintf(stderr, "%" INT64_T_FORMAT "d",
			    (unsigned long long)m->value.q);
			break;
		case FILE_PSTRING:
		case FILE_STRING:
		case FILE_REGEX:
		case FILE_BESTRING16:
		case FILE_LESTRING16:
		case FILE_SEARCH:
			file_showstr(stderr, m->value.s, (size_t)m->vallen);
			break;
		case FILE_DATE:
		case FILE_LEDATE:
		case FILE_BEDATE:
		case FILE_MEDATE:
			(void)fprintf(stderr, "%s,",
			    file_fmttime(m->value.l, 1));
			break;
		case FILE_LDATE:
		case FILE_LELDATE:
		case FILE_BELDATE:
		case FILE_MELDATE:
			(void)fprintf(stderr, "%s,",
			    file_fmttime(m->value.l, 0));
			break;
		case FILE_QDATE:
		case FILE_LEQDATE:
		case FILE_BEQDATE:
			(void)fprintf(stderr, "%s,",
			    file_fmttime((uint32_t)m->value.q, 1));
			break;
		case FILE_QLDATE:
		case FILE_LEQLDATE:
		case FILE_BEQLDATE:
			(void)fprintf(stderr, "%s,",
			    file_fmttime((uint32_t)m->value.q, 0));
			break;
		case FILE_FLOAT:
		case FILE_BEFLOAT:
		case FILE_LEFLOAT:
			(void) fprintf(stderr, "%G", m->value.f);
			break;
		case FILE_DOUBLE:
		case FILE_BEDOUBLE:
		case FILE_LEDOUBLE:
			(void) fprintf(stderr, "%G", m->value.d);
			break;
		case FILE_DEFAULT:
			/* XXX - do anything here? */
			break;
		default:
			(void) fputs("*bad*", stderr);
			break;
		}
	}
	(void) fprintf(stderr, ",\"%s\"]\n", m->desc);
}
#endif

/*VARARGS*/
protected void
file_magwarn(struct magic_set *ms, const char *f, ...)
{
	va_list va;

	/* cuz we use stdout for most, stderr here */
	(void) fflush(stdout); 

	if (ms->file)
		(void) fprintf(stderr, "%s, %lu: ", ms->file,
		    (unsigned long)ms->line);
	(void) fprintf(stderr, "Warning: ");
	va_start(va, f);
	(void) vfprintf(stderr, f, va);
	va_end(va);
	(void) fputc('\n', stderr);
}

protected const char *
file_fmttime(uint32_t v, int local)
{
	char *pp;
	time_t t = (time_t)v;
	struct tm *tm;

	if (local) {
		pp = ctime(&t);
	} else {
#ifndef HAVE_DAYLIGHT
		private int daylight = 0;
#ifdef HAVE_TM_ISDST
		private time_t now = (time_t)0;

		if (now == (time_t)0) {
			struct tm *tm1;
			(void)time(&now);
			tm1 = localtime(&now);
			if (tm1 == NULL)
				goto out;
			daylight = tm1->tm_isdst;
		}
#endif /* HAVE_TM_ISDST */
#endif /* HAVE_DAYLIGHT */
		if (daylight)
			t += 3600;
		tm = gmtime(&t);
		if (tm == NULL)
			goto out;
		pp = asctime(tm);
	}

	if (pp == NULL)
		goto out;
	pp[strcspn(pp, "\n")] = '\0';
	return pp;
out:
	return "*Invalid time*";
}