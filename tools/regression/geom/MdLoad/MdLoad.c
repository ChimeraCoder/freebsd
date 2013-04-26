
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
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <paths.h>
#include <fcntl.h>
#include <err.h>
#include <bsdxml.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <sys/sbuf.h>
#include <sys/mman.h>

struct sector {
	LIST_ENTRY(sector)	sectors;
	off_t			offset;
	unsigned char		*data;
};

struct simdisk_softc {
	int			sectorsize;
	off_t			mediasize;
	off_t			lastsector;
	LIST_HEAD(,sector)	sectors;
	struct sbuf		*sbuf;
	struct sector		*sp;
	u_int			fwsectors;
	u_int			fwheads;
	u_int			fwcylinders;
};

static void
g_simdisk_insertsector(struct simdisk_softc *sc, struct sector *dsp)
{
	struct sector *dsp2, *dsp3;

	if (sc->lastsector < dsp->offset)
		sc->lastsector = dsp->offset;
	if (LIST_EMPTY(&sc->sectors)) {
		LIST_INSERT_HEAD(&sc->sectors, dsp, sectors);
		return;
	}
	dsp3 = NULL;
	LIST_FOREACH(dsp2, &sc->sectors, sectors) {
		dsp3 = dsp2;
		if (dsp2->offset > dsp->offset) {
			LIST_INSERT_BEFORE(dsp2, dsp, sectors);
			return;
		}
	}
	LIST_INSERT_AFTER(dsp3, dsp, sectors);
}

static void
startElement(void *userData, const char *name, const char **atts __unused)
{
	struct simdisk_softc *sc;

	sc = userData;
	if (!strcasecmp(name, "sector")) {
		sc->sp = calloc(1, sizeof(*sc->sp) + sc->sectorsize);
		sc->sp->data = (u_char *)(sc->sp + 1);
	}
	sbuf_clear(sc->sbuf);
}

static void
endElement(void *userData, const char *name)
{
	struct simdisk_softc *sc;
	char *p;
	u_char *q;
	int i, j;
	off_t o;

	sc = userData;

	if (!strcasecmp(name, "comment")) {
		sbuf_clear(sc->sbuf);
		return;
	}
	sbuf_finish(sc->sbuf);
	if (!strcasecmp(name, "sectorsize")) {
		sc->sectorsize = strtoul(sbuf_data(sc->sbuf), &p, 0);
		if (*p != '\0')
			errx(1, "strtoul croaked on sectorsize");
	} else if (!strcasecmp(name, "mediasize")) {
		o = strtoull(sbuf_data(sc->sbuf), &p, 0);
		if (*p != '\0')
			errx(1, "strtoul croaked on mediasize");
		if (o > 0)
			sc->mediasize = o;
	} else if (!strcasecmp(name, "fwsectors")) {
		sc->fwsectors = strtoul(sbuf_data(sc->sbuf), &p, 0);
		if (*p != '\0')
			errx(1, "strtoul croaked on fwsectors");
	} else if (!strcasecmp(name, "fwheads")) {
		sc->fwheads = strtoul(sbuf_data(sc->sbuf), &p, 0);
		if (*p != '\0')
			errx(1, "strtoul croaked on fwheads");
	} else if (!strcasecmp(name, "fwcylinders")) {
		sc->fwcylinders = strtoul(sbuf_data(sc->sbuf), &p, 0);
		if (*p != '\0')
			errx(1, "strtoul croaked on fwcylinders");
	} else if (!strcasecmp(name, "offset")) {
		sc->sp->offset= strtoull(sbuf_data(sc->sbuf), &p, 0);
		if (*p != '\0')
			errx(1, "strtoul croaked on offset");
	} else if (!strcasecmp(name, "fill")) {
		j = strtoul(sbuf_data(sc->sbuf), NULL, 16);
		memset(sc->sp->data, j, sc->sectorsize);
	} else if (!strcasecmp(name, "hexdata")) {
		q = sc->sp->data;
		p = sbuf_data(sc->sbuf);
		for (i = 0; i < sc->sectorsize; i++) {
			if (!isxdigit(*p))
				errx(1, "I croaked on hexdata %d:(%02x)", i, *p);
			if (isdigit(*p))
				j = (*p - '0') << 4;
			else
				j = (tolower(*p) - 'a' + 10) << 4;
			p++;
			if (!isxdigit(*p))
				errx(1, "I croaked on hexdata %d:(%02x)", i, *p);
			if (isdigit(*p))
				j |= *p - '0';
			else
				j |= tolower(*p) - 'a' + 10;
			p++;
			*q++ = j;
		}
	} else if (!strcasecmp(name, "sector")) {
		g_simdisk_insertsector(sc, sc->sp);
		sc->sp = NULL;
	} else if (!strcasecmp(name, "diskimage")) {
	} else if (!strcasecmp(name, "FreeBSD")) {
	} else {
		printf("<%s>[[%s]]\n", name, sbuf_data(sc->sbuf));
	}
	sbuf_clear(sc->sbuf);
}

static void
characterData(void *userData, const XML_Char *s, int len)
{
	const char *b, *e;
	struct simdisk_softc *sc;

	sc = userData;
	b = s;
	e = s + len - 1;
	while (isspace(*b) && b < e)
		b++;
	while (isspace(*e) && e > b)
		e--;
	if (e != b || !isspace(*b))
		sbuf_bcat(sc->sbuf, b, e - b + 1);
}

static struct simdisk_softc *
g_simdisk_xml_load(const char *file)
{
	XML_Parser parser = XML_ParserCreate(NULL);
	struct stat st;
	char *p;
	struct simdisk_softc *sc;
	int fd, i;

	sc = calloc(1, sizeof *sc);
	sc->sbuf = sbuf_new_auto();
	LIST_INIT(&sc->sectors);
	XML_SetUserData(parser, sc);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, characterData);

	fd = open(file, O_RDONLY);
	if (fd < 0)
		err(1, file);
	fstat(fd, &st);
	p = mmap(NULL, st.st_size, PROT_READ, MAP_NOCORE|MAP_PRIVATE, fd, 0);
	i = XML_Parse(parser, p, st.st_size, 1);
	if (i != 1)
		errx(1, "XML_Parse complains: return %d", i);
	munmap(p, st.st_size);
	close(fd);
	XML_ParserFree(parser);
	return (sc);
}

int
main(int argc, char **argv)
{
	struct simdisk_softc *sc;
	char buf[BUFSIZ];
	int error, fd;
	struct sector *dsp;

	if (argc != 3)
		errx(1, "Usage: %s mddevice xmlfile", argv[0]);

	sc = g_simdisk_xml_load(argv[2]);
	if (sc->mediasize == 0)
		sc->mediasize = sc->lastsector + sc->sectorsize * 10;
	if (sc->sectorsize == 0)
		sc->sectorsize = 512;
	sprintf(buf, "mdconfig -a -t malloc -s %jd -S %d",
	    (intmax_t)sc->mediasize / sc->sectorsize, sc->sectorsize);
	if (sc->fwsectors && sc->fwheads)
		sprintf(buf + strlen(buf), " -x %d -y %d",
		    sc->fwsectors, sc->fwheads);
	sprintf(buf + strlen(buf), " -u %s", argv[1]);
	error = system(buf);
	if (error)
		return (error);
	fd = open(argv[1], O_RDWR);
	if (fd < 0 && errno == ENOENT) {
		sprintf(buf, "%s%s", _PATH_DEV, argv[1]);
		fd = open(buf, O_RDWR);
	}
	if (fd < 0)
		err(1, "Could not open %s", argv[1]);
	LIST_FOREACH(dsp, &sc->sectors, sectors) {
		lseek(fd, dsp->offset, SEEK_SET);
		error = write(fd, dsp->data, sc->sectorsize);
		if (error != sc->sectorsize)
			err(1, "write sectordata failed");
	}
	close(fd);
	exit (0);
}