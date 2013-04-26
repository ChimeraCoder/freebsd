
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

#include <dev/hpt27xx/hpt27xx_config.h>
/****************************************************************************
 * config.c - auto-generated file
 ****************************************************************************/
#include <dev/hpt27xx/os_bsd.h>

extern int init_module_him_rr2720(void);
extern int init_module_him_rr273x(void);
extern int init_module_him_rr276x(void);
extern int init_module_him_rr278x(void);
extern int init_module_vdev_raw(void);
extern int init_module_partition(void);
extern int init_module_raid0(void);
extern int init_module_raid1(void);
extern int init_module_raid5(void);
extern int init_module_jbod(void);

int init_config(void)
{
	init_module_him_rr2720();
	init_module_him_rr273x();
	init_module_him_rr276x();
	init_module_him_rr278x();
	init_module_vdev_raw();
	init_module_partition();
	init_module_raid0();
	init_module_raid1();
	init_module_raid5();
	init_module_jbod();
	return 0;
}

const char driver_name[] = "hpt27xx";
const char driver_name_long[] = "RocketRAID 27xx controller driver";
const char driver_ver[] = "v1.0";
int  osm_max_targets = 0xff;


int os_max_cache_size = 0x1000000;