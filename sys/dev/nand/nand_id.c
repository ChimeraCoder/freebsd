
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

#include <dev/nand/nand.h>

struct nand_params nand_ids[] = {
	{ { NAND_MAN_SAMSUNG, 0x75 }, "Samsung K9F5608U0B",
	    0x20, 0x200, 0x10, 0x20, 0 },
	{ { NAND_MAN_SAMSUNG, 0xd3 }, "Samsung NAND 1GiB 3,3V 8-bit",
	    0x400, 0x800, 0x40, 0x40, 0 },
	{ { NAND_MAN_SAMSUNG, 0xdc }, "Samsung NAND 512MiB 3,3V 8-bit",
	    0x200, 0x800, 0x40, 0x40, 0 },
	{ { NAND_MAN_HYNIX, 0x76 }, "Hynix NAND 64MiB 3,3V 8-bit",
	    0x40, 0x200, 0x10, 0x20, 0 },
	{ { NAND_MAN_HYNIX, 0xdc }, "Hynix NAND 512MiB 3,3V 8-bit",
	    0x200, 0x800, 0x40, 0x40, 0 },
	{ { NAND_MAN_HYNIX, 0x79 }, "NAND 128MB 3,3V 8-bit",
	    0x80, 0x200, 0x10, 0x20, 0 },
	{ { NAND_MAN_STMICRO, 0xf1 }, "STMicro 128MB 3,3V 8-bit",
	    0x80, 2048, 64, 0x40, 0 },
};

struct nand_params *nand_get_params(struct nand_id *id)
{
	int i;

	for (i = 0; i < sizeof(nand_ids) / sizeof(nand_ids[0]); i++)
		if (nand_ids[i].id.man_id == id->man_id &&
		    nand_ids[i].id.dev_id == id->dev_id)
			return (&nand_ids[i]);

	return (NULL);
}