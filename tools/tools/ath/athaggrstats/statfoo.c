
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

#include <stdio.h>
#include <string.h>

#include "statfoo.h"

static void
statfoo_setfmt(struct statfoo *sf, const char *fmt0)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	char fmt[4096];
	char *fp, *tok;
	int i, j;

	j = 0;
	strlcpy(fmt, fmt0, sizeof(fmt));
	for (fp = fmt; (tok = strsep(&fp, ", ")) != NULL;) {
		for (i = 0; i < sf->nstats; i++)
			if (strcasecmp(tok, sf->stats[i].name) == 0)
				break;
		if (i >= sf->nstats) {
			fprintf(stderr, "%s: unknown statistic name \"%s\" "
				"skipped\n", sf->name, tok);
			continue;
		}
		if (j+3 > sizeof(sf->fmts)) {
			fprintf(stderr, "%s: not enough room for all stats; "
				"stopped at %s\n", sf->name, tok);
			break;
		}
		if (j != 0)
			sf->fmts[j++] = ' ';
		sf->fmts[j++] = FMTS_IS_STAT;
		sf->fmts[j++] = i & 0xff;
		sf->fmts[j++] = (i >> 8) & 0xff;
	}
	sf->fmts[j] = '\0';
#undef N
}

static void 
statfoo_collect(struct statfoo *sf)
{
	fprintf(stderr, "%s: don't know how to collect data\n", sf->name);
}

static void 
statfoo_update_tot(struct statfoo *sf)
{
	fprintf(stderr, "%s: don't know how to update total data\n", sf->name);
}

static int 
statfoo_get(struct statfoo *sf, int s, char b[], size_t bs)
{
	fprintf(stderr, "%s: don't know how to get stat #%u\n", sf->name, s);
	return 0;
}

static void
statfoo_print_header(struct statfoo *sf, FILE *fd)
{
	const unsigned char *cp;
	int i;
	const struct fmt *f;

	for (cp = sf->fmts; *cp != '\0'; cp++) {
		if (*cp == FMTS_IS_STAT) {
			i = *(++cp);
			i |= ((int) *(++cp)) << 8;
			f = &sf->stats[i];
			fprintf(fd, "%*s", f->width, f->label);
		} else
			putc(*cp, fd);
	}
	putc('\n', fd);
}

static void
statfoo_print_current(struct statfoo *sf, FILE *fd)
{
	char buf[32];
	const unsigned char *cp;
	int i;
	const struct fmt *f;

	for (cp = sf->fmts; *cp != '\0'; cp++) {
		if (*cp == FMTS_IS_STAT) {
			i = *(++cp);
			i |= ((int) *(++cp)) << 8;
			f = &sf->stats[i];
			if (sf->get_curstat(sf, i, buf, sizeof(buf)))
				fprintf(fd, "%*s", f->width, buf);
		} else
			putc(*cp, fd);
	}
	putc('\n', fd);
}

static void
statfoo_print_total(struct statfoo *sf, FILE *fd)
{
	char buf[32];
	const unsigned char *cp;
	const struct fmt *f;
	int i;

	for (cp = sf->fmts; *cp != '\0'; cp++) {
		if (*cp == FMTS_IS_STAT) {
			i = *(++cp);
			i |= ((int) *(++cp)) << 8;
			f = &sf->stats[i];
			if (sf->get_totstat(sf, i, buf, sizeof(buf)))
				fprintf(fd, "%*s", f->width, buf);
		} else
			putc(*cp, fd);
	}
	putc('\n', fd);
}

static void
statfoo_print_verbose(struct statfoo *sf, FILE *fd)
{
	const struct fmt *f;
	char s[32];
	int i, width;

	width = 0;
	for (i = 0; i < sf->nstats; i++) {
		f = &sf->stats[i];
		if (f->width > width)
			width = f->width;
	}
	for (i = 0; i < sf->nstats; i++) {
		f = &sf->stats[i];
		if (sf->get_totstat(sf, i, s, sizeof(s)) && strcmp(s, "0"))
			fprintf(fd, "%-*s %s\n", width, s, f->desc);
	}
}

static void
statfoo_print_fields(struct statfoo *sf, FILE *fd)
{
	int i, w, width;

	width = 0;
	for (i = 0; i < sf->nstats; i++) {
		w = strlen(sf->stats[i].name);
		if (w > width)
			width = w;
	}
	for (i = 0; i < sf->nstats; i++) {
		const struct fmt *f = &sf->stats[i];
		if (f->width != 0)
			fprintf(fd, "%-*s %s\n", width, f->name, f->desc);
	}
}

void
statfoo_init(struct statfoo *sf, const char *name, const struct fmt *stats, int nstats)
{
	sf->name = name;
	sf->stats = stats;
	sf->nstats = nstats;
	sf->setfmt = statfoo_setfmt;
	sf->collect_cur = statfoo_collect;
	sf->collect_tot = statfoo_collect;
	sf->update_tot = statfoo_update_tot;
	sf->get_curstat = statfoo_get;
	sf->get_totstat = statfoo_get;
	sf->print_header = statfoo_print_header;
	sf->print_current = statfoo_print_current;
	sf->print_total = statfoo_print_total;
	sf->print_verbose = statfoo_print_verbose;
	sf->print_fields = statfoo_print_fields;
}