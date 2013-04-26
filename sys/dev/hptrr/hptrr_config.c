
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
#include <dev/hptrr/hptrr_config.h>
/****************************************************************************
 * config.c - auto-generated file
 ****************************************************************************/
#include <dev/hptrr/os_bsd.h>

extern int init_module_him_rr2310pm(void);
extern int init_module_him_rr174x_rr2210pm(void);
extern int init_module_him_rr2522pm(void);
extern int init_module_him_rr2340(void);
extern int init_module_him_rr222x_rr2240(void);
extern int init_module_him_rr1720(void);
extern int init_module_him_rr232x(void);
extern int init_module_vdev_raw(void);
extern int init_module_partition(void);
extern int init_module_raid0(void);
extern int init_module_raid1(void);
extern int init_module_raid5(void);
extern int init_module_jbod(void);

int init_config(void)
{
	init_module_him_rr2310pm();
	init_module_him_rr174x_rr2210pm();
	init_module_him_rr2522pm();
	init_module_him_rr2340();
	init_module_him_rr222x_rr2240();
	init_module_him_rr1720();
	init_module_him_rr232x();
	init_module_vdev_raw();
	init_module_partition();
	init_module_raid0();
	init_module_raid1();
	init_module_raid5();
	init_module_jbod();
	return 0;
}

char driver_name[] = "hptrr";
char driver_name_long[] = "RocketRAID 17xx/2xxx SATA controller driver";
char driver_ver[] = "v1.2";
int  osm_max_targets = 0xff;


int os_max_cache_size = 0x1000000;