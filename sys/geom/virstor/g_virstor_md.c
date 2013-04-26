
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
#include <sys/endian.h>

#include <geom/virstor/g_virstor_md.h>
#include <geom/virstor/binstream.h>

/*
 * Encode data from g_virstor_metadata structure into a endian-independant
 * byte stream.
 */
void
virstor_metadata_encode(struct g_virstor_metadata *md, unsigned char *data)
{
	bin_stream_t bs;

	bs_open(&bs, data);

	bs_write_buf(&bs, md->md_magic, sizeof(md->md_magic));
	bs_write_u32(&bs, md->md_version);
	bs_write_buf(&bs, md->md_name, sizeof(md->md_name));
	bs_write_u64(&bs, md->md_virsize);
	bs_write_u32(&bs, md->md_chunk_size);
	bs_write_u32(&bs, md->md_id);
	bs_write_u16(&bs, md->md_count);

	bs_write_buf(&bs, md->provider, sizeof(md->provider));
	bs_write_u16(&bs, md->no);
	bs_write_u64(&bs, md->provsize);
	bs_write_u32(&bs, md->chunk_count);
	bs_write_u32(&bs, md->chunk_next);
	bs_write_u16(&bs, md->chunk_reserved);
	bs_write_u16(&bs, md->flags);
}


/*
 * Decode data from endian-independant byte stream into g_virstor_metadata
 * structure.
 */
void
virstor_metadata_decode(unsigned char *data, struct g_virstor_metadata *md)
{
	bin_stream_t bs;

	bs_open(&bs, (char *)(data));

	bs_read_buf(&bs, md->md_magic, sizeof(md->md_magic));
	md->md_version = bs_read_u32(&bs);
	bs_read_buf(&bs, md->md_name, sizeof(md->md_name));
	md->md_virsize = bs_read_u64(&bs);
	md->md_chunk_size = bs_read_u32(&bs);
	md->md_id = bs_read_u32(&bs);
	md->md_count = bs_read_u16(&bs);

	bs_read_buf(&bs, md->provider, sizeof(md->provider));
	md->no = bs_read_u16(&bs);
	md->provsize = bs_read_u64(&bs);
	md->chunk_count = bs_read_u32(&bs);
	md->chunk_next = bs_read_u32(&bs);
	md->chunk_reserved = bs_read_u16(&bs);
	md->flags = bs_read_u16(&bs);
}