
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

#include <stand.h>

#include "lv1call.h"
#include "ps3.h"
#include "ps3repo.h"

static uint64_t make_n1(const char *text, unsigned int index)
{
	uint64_t n1;

	n1 = 0;
	strncpy((char *) &n1, text, sizeof(n1));
	n1 = (n1 >> 32) + index;

	return n1;
}

static uint64_t make_n(const char *text, unsigned int index)
{
	uint64_t n;

	n = 0;
	strncpy((char *) &n, text, sizeof(n));
	n = n + index;

	return n;
}

int ps3repo_read_bus_type(unsigned int bus_index, uint64_t *bus_type)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("type", 0), 0, 0, &v1, &v2);

	*bus_type = v1;

	return err;
}

int ps3repo_read_bus_id(unsigned int bus_index, uint64_t *bus_id)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("id", 0), 0, 0, &v1, &v2);

	*bus_id = v1;

	return err;
}

int ps3repo_read_bus_num_dev(unsigned int bus_index, uint64_t *num_dev)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("num_dev", 0), 0, 0, &v1, &v2);

	*num_dev = v1;

	return err;
}

int ps3repo_read_bus_dev_type(unsigned int bus_index, unsigned int dev_index, uint64_t *dev_type)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("dev", dev_index), make_n("type", 0), 0, &v1, &v2);

	*dev_type = v1;

	return err;
}

int ps3repo_read_bus_dev_id(unsigned int bus_index, unsigned int dev_index, uint64_t *dev_id)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("dev", dev_index), make_n("id", 0), 0, &v1, &v2);

	*dev_id = v1;

	return err;
}

int ps3repo_read_bus_dev_blk_size(unsigned int bus_index, unsigned int dev_index, uint64_t *blk_size)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("dev", dev_index), make_n("blk_size", 0), 0, &v1, &v2);

	*blk_size = v1;

	return err;
}

int ps3repo_read_bus_dev_nblocks(unsigned int bus_index, unsigned int dev_index, uint64_t *nblocks)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("dev", dev_index), make_n("n_blocks", 0), 0, &v1, &v2);

	*nblocks = v1;

	return err;
}

int ps3repo_read_bus_dev_nregs(unsigned int bus_index, unsigned int dev_index, uint64_t *nregs)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("dev", dev_index), make_n("n_regs", 0), 0, &v1, &v2);

	*nregs = v1;

	return err;
}

int ps3repo_read_bus_dev_reg_id(unsigned int bus_index, unsigned int dev_index,
	unsigned int reg_index, uint64_t *reg_id)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("dev", dev_index), make_n("region", reg_index), make_n("id", 0), &v1, &v2);

	*reg_id = v1;

	return err;
}

int ps3repo_read_bus_dev_reg_start(unsigned int bus_index, unsigned int dev_index,
	unsigned int reg_index, uint64_t *reg_start)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("dev", dev_index), make_n("region", reg_index), make_n("start", 0), &v1, &v2);

	*reg_start = v1;

	return err;
}

int ps3repo_read_bus_dev_reg_size(unsigned int bus_index, unsigned int dev_index,
	unsigned int reg_index, uint64_t *reg_size)
{
	uint64_t v1, v2;
	int err;

	err = lv1_get_repository_node_value(PS3_LPAR_ID_PME, make_n1("bus", bus_index),
		make_n("dev", dev_index), make_n("region", reg_index), make_n("size", 0), &v1, &v2);

	*reg_size = v1;

	return err;
}

int ps3repo_find_bus_by_type(uint64_t bus_type, unsigned int *bus_index)
{
	unsigned int i;
	uint64_t type;
	int err;

	for (i = 0; i < 10; i++) {
		err = ps3repo_read_bus_type(i, &type);
		if (err) {
			*bus_index = (unsigned int) -1;
			return err;
		}

		if (type == bus_type) {
			*bus_index = i;
			return 0;
		}
	}

	*bus_index = (unsigned int) -1;

	return ENODEV;
}

int ps3repo_find_bus_dev_by_type(unsigned int bus_index, uint64_t dev_type,
	unsigned int *dev_index)
{
	unsigned int i;
	uint64_t type;
	int err;

	for (i = 0; i < 10; i++) {
		err = ps3repo_read_bus_dev_type(bus_index, i, &type);
		if (err) {
			*dev_index = (unsigned int) -1;
			return err;
		}

		if (type == dev_type) {
			*dev_index = i;
			return 0;
		}
	}

	*dev_index = (unsigned int) -1;

	return ENODEV;
}