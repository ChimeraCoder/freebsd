
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>

#include <endian.h>
#include <byteswap.h>

#include <infiniband/verbs.h>
#include <infiniband/arch.h>

int main(int argc, char *argv[])
{
	struct ibv_device **dev_list;
	int num_devices, i;

	dev_list = ibv_get_device_list(&num_devices);
	if (!dev_list) {
		perror("Failed to get IB devices list");
		return 1;
	}

	printf("    %-16s\t   node GUID\n", "device");
	printf("    %-16s\t----------------\n", "------");

	for (i = 0; i < num_devices; ++i) {
		printf("    %-16s\t%016llx\n",
		       ibv_get_device_name(dev_list[i]),
		       (unsigned long long) ntohll(ibv_get_device_guid(dev_list[i])));
	}

	ibv_free_device_list(dev_list);

	return 0;
}