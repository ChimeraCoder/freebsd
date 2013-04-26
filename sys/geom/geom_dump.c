
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
#include <sys/sbuf.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <machine/stdarg.h>

#include <geom/geom.h>
#include <geom/geom_int.h>
#include <geom/geom_disk.h>


static void
g_confdot_consumer(struct sbuf *sb, struct g_consumer *cp)
{

	sbuf_printf(sb, "z%p [label=\"r%dw%de%d\"];\n",
	    cp, cp->acr, cp->acw, cp->ace);
	if (cp->provider)
		sbuf_printf(sb, "z%p -> z%p;\n", cp, cp->provider);
}

static void
g_confdot_provider(struct sbuf *sb, struct g_provider *pp)
{

	sbuf_printf(sb, "z%p [shape=hexagon,label=\"%s\\nr%dw%de%d\\nerr#%d\"];\n",
	    pp, pp->name, pp->acr, pp->acw, pp->ace, pp->error);
}

static void
g_confdot_geom(struct sbuf *sb, struct g_geom *gp)
{
	struct g_consumer *cp;
	struct g_provider *pp;

	sbuf_printf(sb, "z%p [shape=box,label=\"%s\\n%s\\nr#%d\"];\n",
	    gp, gp->class->name, gp->name, gp->rank);
	LIST_FOREACH(cp, &gp->consumer, consumer) {
		g_confdot_consumer(sb, cp);
		sbuf_printf(sb, "z%p -> z%p;\n", gp, cp);
	}

	LIST_FOREACH(pp, &gp->provider, provider) {
		g_confdot_provider(sb, pp);
		sbuf_printf(sb, "z%p -> z%p;\n", pp, gp);
	}
}

static void
g_confdot_class(struct sbuf *sb, struct g_class *mp)
{
	struct g_geom *gp;

	LIST_FOREACH(gp, &mp->geom, geom)
		g_confdot_geom(sb, gp);
}

void
g_confdot(void *p, int flag )
{
	struct g_class *mp;
	struct sbuf *sb;

	KASSERT(flag != EV_CANCEL, ("g_confdot was cancelled"));
	sb = p;
	g_topology_assert();
	sbuf_printf(sb, "digraph geom {\n");
	LIST_FOREACH(mp, &g_classes, class)
		g_confdot_class(sb, mp);
	sbuf_printf(sb, "};\n");
	sbuf_finish(sb);
}

static void
g_conftxt_geom(struct sbuf *sb, struct g_geom *gp, int level)
{
	struct g_provider *pp;
	struct g_consumer *cp;

	if (gp->flags & G_GEOM_WITHER)
		return;
	LIST_FOREACH(pp, &gp->provider, provider) {
		sbuf_printf(sb, "%d %s %s %ju %u", level, gp->class->name,
		    pp->name, (uintmax_t)pp->mediasize, pp->sectorsize);
		if (gp->dumpconf != NULL)
			gp->dumpconf(sb, NULL, gp, NULL, pp);
		sbuf_printf(sb, "\n");
		LIST_FOREACH(cp, &pp->consumers, consumers)
			g_conftxt_geom(sb, cp->geom, level + 1);
	}
}

static void
g_conftxt_class(struct sbuf *sb, struct g_class *mp)
{
	struct g_geom *gp;

	LIST_FOREACH(gp, &mp->geom, geom)
		g_conftxt_geom(sb, gp, 0);
}

void
g_conftxt(void *p, int flag)
{
	struct g_class *mp;
	struct sbuf *sb;

	KASSERT(flag != EV_CANCEL, ("g_conftxt was cancelled"));
	sb = p;
	g_topology_assert();
	LIST_FOREACH(mp, &g_classes, class) {
		if (!strcmp(mp->name, G_DISK_CLASS_NAME) || !strcmp(mp->name, "MD"))
			g_conftxt_class(sb, mp);
	}
	sbuf_finish(sb);
}


static void
g_conf_print_escaped(struct sbuf *sb, const char *fmt, const char *str)
{
	struct sbuf *s;
	const u_char *c;

	s = sbuf_new_auto();

	for (c = str; *c != '\0'; c++) {
		if (*c == '&' || *c == '<' || *c == '>' ||
		    *c == '\'' || *c == '"' || *c > 0x7e)
			sbuf_printf(s, "&#x%X;", *c);
		else if (*c == '\t' || *c == '\n' || *c == '\r' || *c > 0x1f)
			sbuf_putc(s, *c);
		else
			sbuf_putc(s, '?');
	}
	sbuf_finish(s);
	sbuf_printf(sb, fmt, sbuf_data(s));
	sbuf_delete(s);
}

static void
g_conf_consumer(struct sbuf *sb, struct g_consumer *cp)
{

	sbuf_printf(sb, "\t<consumer id=\"%p\">\n", cp);
	sbuf_printf(sb, "\t  <geom ref=\"%p\"/>\n", cp->geom);
	if (cp->provider != NULL)
		sbuf_printf(sb, "\t  <provider ref=\"%p\"/>\n", cp->provider);
	sbuf_printf(sb, "\t  <mode>r%dw%de%d</mode>\n",
	    cp->acr, cp->acw, cp->ace);
	if (cp->geom->flags & G_GEOM_WITHER)
		;
	else if (cp->geom->dumpconf != NULL) {
		sbuf_printf(sb, "\t  <config>\n");
		cp->geom->dumpconf(sb, "\t    ", cp->geom, cp, NULL);
		sbuf_printf(sb, "\t  </config>\n");
	}
	sbuf_printf(sb, "\t</consumer>\n");
}

static void
g_conf_provider(struct sbuf *sb, struct g_provider *pp)
{

	sbuf_printf(sb, "\t<provider id=\"%p\">\n", pp);
	sbuf_printf(sb, "\t  <geom ref=\"%p\"/>\n", pp->geom);
	sbuf_printf(sb, "\t  <mode>r%dw%de%d</mode>\n",
	    pp->acr, pp->acw, pp->ace);
	g_conf_print_escaped(sb, "\t  <name>%s</name>\n", pp->name);
	sbuf_printf(sb, "\t  <mediasize>%jd</mediasize>\n",
	    (intmax_t)pp->mediasize);
	sbuf_printf(sb, "\t  <sectorsize>%u</sectorsize>\n", pp->sectorsize);
	sbuf_printf(sb, "\t  <stripesize>%u</stripesize>\n", pp->stripesize);
	sbuf_printf(sb, "\t  <stripeoffset>%u</stripeoffset>\n", pp->stripeoffset);
	if (pp->geom->flags & G_GEOM_WITHER)
		;
	else if (pp->geom->dumpconf != NULL) {
		sbuf_printf(sb, "\t  <config>\n");
		pp->geom->dumpconf(sb, "\t    ", pp->geom, NULL, pp);
		sbuf_printf(sb, "\t  </config>\n");
	}
	sbuf_printf(sb, "\t</provider>\n");
}


static void
g_conf_geom(struct sbuf *sb, struct g_geom *gp, struct g_provider *pp, struct g_consumer *cp)
{
	struct g_consumer *cp2;
	struct g_provider *pp2;

	sbuf_printf(sb, "    <geom id=\"%p\">\n", gp);
	sbuf_printf(sb, "      <class ref=\"%p\"/>\n", gp->class);
	g_conf_print_escaped(sb, "      <name>%s</name>\n", gp->name);
	sbuf_printf(sb, "      <rank>%d</rank>\n", gp->rank);
	if (gp->flags & G_GEOM_WITHER)
		sbuf_printf(sb, "      <wither/>\n");
	else if (gp->dumpconf != NULL) {
		sbuf_printf(sb, "      <config>\n");
		gp->dumpconf(sb, "\t", gp, NULL, NULL);
		sbuf_printf(sb, "      </config>\n");
	}
	LIST_FOREACH(cp2, &gp->consumer, consumer) {
		if (cp != NULL && cp != cp2)
			continue;
		g_conf_consumer(sb, cp2);
	}

	LIST_FOREACH(pp2, &gp->provider, provider) {
		if (pp != NULL && pp != pp2)
			continue;
		g_conf_provider(sb, pp2);
	}
	sbuf_printf(sb, "    </geom>\n");
}

static void
g_conf_class(struct sbuf *sb, struct g_class *mp, struct g_geom *gp, struct g_provider *pp, struct g_consumer *cp)
{
	struct g_geom *gp2;

	sbuf_printf(sb, "  <class id=\"%p\">\n", mp);
	g_conf_print_escaped(sb, "    <name>%s</name>\n", mp->name);
	LIST_FOREACH(gp2, &mp->geom, geom) {
		if (gp != NULL && gp != gp2)
			continue;
		g_conf_geom(sb, gp2, pp, cp);
	}
	sbuf_printf(sb, "  </class>\n");
}

void
g_conf_specific(struct sbuf *sb, struct g_class *mp, struct g_geom *gp, struct g_provider *pp, struct g_consumer *cp)
{
	struct g_class *mp2;

	g_topology_assert();
	sbuf_printf(sb, "<mesh>\n");
	LIST_FOREACH(mp2, &g_classes, class) {
		if (mp != NULL && mp != mp2)
			continue;
		g_conf_class(sb, mp2, gp, pp, cp);
	}
	sbuf_printf(sb, "</mesh>\n");
	sbuf_finish(sb);
}

void
g_confxml(void *p, int flag)
{

	KASSERT(flag != EV_CANCEL, ("g_confxml was cancelled"));
	g_topology_assert();
	g_conf_specific(p, NULL, NULL, NULL, NULL);
}

void
g_trace(int level, const char *fmt, ...)
{
	va_list ap;

	if (!(g_debugflags & level))
		return;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}