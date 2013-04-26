
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "misc.h"
#include "queue.h"
#include "rcsfile.h"
#include "rcsparse.h"
#include "rcstokenizer.h"

/*
 * This is an RCS-parser using lex for tokenizing and makes sure the RCS syntax
 * is correct as it constructs an RCS file that is used by csup.
 */

static void	asserttoken(yyscan_t *, int);
static int	parse_admin(struct rcsfile *, yyscan_t *);
static int	parse_deltas(struct rcsfile *, yyscan_t *, int);
static int	parse_deltatexts(struct rcsfile *, yyscan_t *, int);
static char	*duptext(yyscan_t *, int *);

struct string {
	char *str;
	STAILQ_ENTRY(string) next;
};

static void
asserttoken(yyscan_t *sp, int token)
{
	int t;

	t = token;
	t = rcslex(*sp);
	assert(t == token);
}

static char *
duptext(yyscan_t *sp, int *arglen)
{
	char *tmp, *val;
	int len;

	tmp = rcsget_text(*sp);
	len = rcsget_leng(*sp);
	val = xmalloc(len + 1);
	memcpy(val, tmp, len);
	val[len] = '\0';
	if (arglen != NULL)
		*arglen = len;
	return (val);
}

/*
 * Start up parser, and use the rcsfile hook to add objects.
 */
int
rcsparse_run(struct rcsfile *rf, FILE *infp, int ro)
{
	yyscan_t scanner;
	char *desc;
	int error, tok;
	
	error = 0;
	rcslex_init(&scanner);
	rcsset_in(infp, scanner);
	tok = parse_admin(rf, &scanner);
	tok = parse_deltas(rf, &scanner, tok);
	assert(tok == KEYWORD);
	asserttoken(&scanner, STRING);
	desc = duptext(&scanner, NULL);
	rcsfile_setval(rf, RCSFILE_DESC, desc);
	free(desc);
	tok = rcslex(scanner);
	/* Parse deltatexts if we need to edit. */
	if (!ro) {
		error = parse_deltatexts(rf, &scanner, tok);
		if (error)
			return (error);
	}
	rcslex_destroy(scanner);
	return (0);
}

/*
 * Parse the admin part of a RCS file.
 */
static int
parse_admin(struct rcsfile *rf, yyscan_t *sp)
{
	char *branch, *comment, *expand, *head, *id, *revnum, *tag, *tmp;
	int strict, token;

	strict = 0;
	branch = NULL;

	/* head {num}; */
	asserttoken(sp, KEYWORD);
	asserttoken(sp, NUM);
	head = duptext(sp, NULL);
	rcsfile_setval(rf, RCSFILE_HEAD, head);
	free(head);
	asserttoken(sp, SEMIC);

	/* { branch {num}; } */
	token = rcslex(*sp);
	if (token == KEYWORD_TWO) {
		asserttoken(sp, NUM);
		branch = duptext(sp, NULL);
		rcsfile_setval(rf, RCSFILE_BRANCH, branch);
		free(branch);
		asserttoken(sp, SEMIC);
		token = rcslex(*sp);
	}

	/* access {id]*; */
	assert(token == KEYWORD);
	token = rcslex(*sp);
	while (token == ID) {
		id = duptext(sp, NULL);
		rcsfile_addaccess(rf, id);
		free(id);
		token = rcslex(*sp);
	}
	assert(token == SEMIC);

	/* symbols {sym : num}*; */
	asserttoken(sp, KEYWORD);
	token = rcslex(*sp);
	while (token == ID) {
		tag = duptext(sp, NULL);
		asserttoken(sp, COLON);
		asserttoken(sp, NUM);
		revnum = duptext(sp, NULL);
		rcsfile_importtag(rf, tag, revnum);
		free(tag);
		free(revnum);
		token = rcslex(*sp);
	}
	assert(token == SEMIC);

	/* locks {id : num}*; */
	asserttoken(sp, KEYWORD);
	token = rcslex(*sp);
	while (token == ID) {
		/* XXX: locks field is skipped */
		asserttoken(sp, COLON);
		asserttoken(sp, NUM);
		token = rcslex(*sp);
	}
	assert(token == SEMIC);
	token = rcslex(*sp);
	while (token == KEYWORD) {
		tmp = rcsget_text(*sp);

		/* {strict  ;} */
		if (!strcmp(tmp, "strict")) {
			rcsfile_setval(rf, RCSFILE_STRICT, tmp);
			asserttoken(sp, SEMIC);
		/* { comment {string}; } */
		} else if (!strcmp(tmp, "comment")) {
			token = rcslex(*sp);
			if (token == STRING) {
				comment = duptext(sp, NULL);
				rcsfile_setval(rf, RCSFILE_COMMENT, comment);
				free(comment);
			}
			asserttoken(sp, SEMIC);
		/* { expand {string}; } */
		} else if (!strcmp(tmp, "expand")) {
			token = rcslex(*sp);
			if (token == STRING) {
				expand = duptext(sp, NULL);
				rcsfile_setval(rf, RCSFILE_EXPAND, expand);
				free(expand);
			}
			asserttoken(sp, SEMIC);
		} 
		/* {newphrase }* */
		token = rcslex(*sp);
		while (token == ID) {
			token = rcslex(*sp);
			/* XXX: newphrases ignored */
			while (token == ID || token == NUM || token == STRING ||
			    token == COLON) {
				token = rcslex(*sp);
			}
			asserttoken(sp, SEMIC);
			token = rcslex(*sp);
		}
	}
	return (token);
}

/*
 * Parse RCS deltas.
 */
static int
parse_deltas(struct rcsfile *rf, yyscan_t *sp, int token)
{
	STAILQ_HEAD(, string) branchlist;
	char *revnum, *revdate, *author, *state, *next;

	/* In case we don't have deltas. */
	if (token != NUM)
		return (token);
	do {
		next = NULL;
		state = NULL;

		/* num */
		assert(token == NUM);
		revnum = duptext(sp, NULL);
		/* date num; */
		asserttoken(sp, KEYWORD);
		asserttoken(sp, NUM);
		revdate = duptext(sp, NULL);
		asserttoken(sp, SEMIC);
		/* author id; */
		asserttoken(sp, KEYWORD);
		asserttoken(sp, ID);
		author = duptext(sp, NULL);
		asserttoken(sp, SEMIC);
		/* state {id}; */
		asserttoken(sp, KEYWORD);
		token = rcslex(*sp);
		if (token == ID) {
			state = duptext(sp, NULL);
			token = rcslex(*sp);
		}
		assert(token == SEMIC);
		/* branches {num}*; */
		asserttoken(sp, KEYWORD);
		token = rcslex(*sp);
		STAILQ_INIT(&branchlist);
		while (token == NUM)
			token = rcslex(*sp);
		assert(token == SEMIC);
		/* next {num}; */
		asserttoken(sp, KEYWORD);
		token = rcslex(*sp);
		if (token == NUM) {
			next = duptext(sp, NULL);
			token = rcslex(*sp);
		}
		assert(token == SEMIC);
		/* {newphrase }* */
		token = rcslex(*sp);
		while (token == ID) {
			token = rcslex(*sp);
			/* XXX: newphrases ignored. */
			while (token == ID || token == NUM || token == STRING ||
			    token == COLON) {
				token = rcslex(*sp);
			}
			asserttoken(sp, SEMIC);
			token = rcslex(*sp);
		}
		rcsfile_importdelta(rf, revnum, revdate, author, state, next);
		free(revnum);
		free(revdate);
		free(author);
		if (state != NULL)
			free(state);
		if (next != NULL)
			free(next);
	} while (token == NUM);

	return (token);
}

/*
 * Parse RCS deltatexts.
 */
static int
parse_deltatexts(struct rcsfile *rf, yyscan_t *sp, int token)
{
	struct delta *d;
	char *log, *revnum, *text;
	int error, len;

	error = 0;
	/* In case we don't have deltatexts. */
	if (token != NUM)
		return (-1);
	do {
		/* num */
		assert(token == NUM);
		revnum = duptext(sp, NULL);
		/* Get delta we're adding text to. */
		d = rcsfile_getdelta(rf, revnum);
		free(revnum);

		/*
		 * XXX: The RCS file is corrupt, but lie and say it is ok.
		 * If it is actually broken, then the MD5 mismatch will
		 * trigger a fixup.
		 */
		if (d == NULL)
			return (0);

		/* log string */
		asserttoken(sp, KEYWORD);
		asserttoken(sp, STRING);
		log = duptext(sp, &len);
		error = rcsdelta_addlog(d, log, len);
		free(log);
		if (error)
			return (-1);
		/* { newphrase }* */
		token = rcslex(*sp);
		while (token == ID) {
			token = rcslex(*sp);
			/* XXX: newphrases ignored. */
			while (token == ID || token == NUM || token == STRING ||
			    token == COLON) {
				token = rcslex(*sp);
			}
			asserttoken(sp, SEMIC);
			token = rcslex(*sp);
		}
		/* text string */
		assert(token == KEYWORD);
		asserttoken(sp, STRING);
		text = duptext(sp, &len);
		error = rcsdelta_addtext(d, text, len);
		/*
		 * If this happens, something is wrong with the RCS file, and it
		 * should be resent.
		 */
		free(text);
		if (error)
			return (-1);
		token = rcslex(*sp);
	} while (token == NUM);

	return (0);
}