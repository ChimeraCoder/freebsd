
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

#ifdef HAVE_KERNEL_OPTION_HEADERS
#include "opt_snd.h"
#endif

#include <dev/sound/pcm/sound.h>

#include <isa/isavar.h>

SND_DECLARE_FILE("$FreeBSD$");

int
sndbuf_dmasetup(struct snd_dbuf *b, struct resource *drq)
{
	/* should do isa_dma_acquire/isa_dma_release here */
	if (drq == NULL) {
		b->dmachan = -1;
	} else {
		sndbuf_setflags(b, SNDBUF_F_DMA, 1);
		b->dmachan = rman_get_start(drq);
	}
	return 0;
}

int
sndbuf_dmasetdir(struct snd_dbuf *b, int dir)
{
	KASSERT(b, ("sndbuf_dmasetdir called with b == NULL"));
	KASSERT(sndbuf_getflags(b) & SNDBUF_F_DMA, ("sndbuf_dmasetdir called on non-ISA buffer"));

	b->dir = (dir == PCMDIR_PLAY)? ISADMA_WRITE : ISADMA_READ;
	return 0;
}

void
sndbuf_dma(struct snd_dbuf *b, int go)
{
	KASSERT(b, ("sndbuf_dma called with b == NULL"));
	KASSERT(sndbuf_getflags(b) & SNDBUF_F_DMA, ("sndbuf_dma called on non-ISA buffer"));

	switch (go) {
	case PCMTRIG_START:
		/* isa_dmainit(b->chan, size); */
		isa_dmastart(b->dir | ISADMA_RAW, b->buf, b->bufsize, b->dmachan);
		break;

	case PCMTRIG_STOP:
	case PCMTRIG_ABORT:
		isa_dmastop(b->dmachan);
		isa_dmadone(b->dir | ISADMA_RAW, b->buf, b->bufsize, b->dmachan);
		break;
	}

	DEB(printf("buf 0x%p ISA DMA %s, channel %d\n",
		b,
		(go == PCMTRIG_START)? "started" : "stopped",
		b->dmachan));
}

int
sndbuf_dmaptr(struct snd_dbuf *b)
{
	int i;

	KASSERT(b, ("sndbuf_dmaptr called with b == NULL"));
	KASSERT(sndbuf_getflags(b) & SNDBUF_F_DMA, ("sndbuf_dmaptr called on non-ISA buffer"));

	if (!sndbuf_runsz(b))
		return 0;
	i = isa_dmastatus(b->dmachan);
	KASSERT(i >= 0, ("isa_dmastatus returned %d", i));
	return b->bufsize - i;
}

void
sndbuf_dmabounce(struct snd_dbuf *b)
{
	KASSERT(b, ("sndbuf_dmabounce called with b == NULL"));
	KASSERT(sndbuf_getflags(b) & SNDBUF_F_DMA, ("sndbuf_dmabounce called on non-ISA buffer"));

	/* tell isa_dma to bounce data in/out */
}