
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

#include <sys/types.h>
#include <stdio.h>
#include <paths.h>
#include <string.h>

#include <dev/mlx/mlxio.h>
#include <dev/mlx/mlxreg.h>

#include "mlxcontrol.h"

/********************************************************************************
 * Various name-producing and -parsing functions
 */

/* return path of controller (unit) */
char *
ctrlrpath(int unit)
{
    static char	buf[32];
    
    sprintf(buf, "%s%s", _PATH_DEV, ctrlrname(unit));
    return(buf);
}

/* return name of controller (unit) */
char *
ctrlrname(int unit)
{
    static char	buf[32];
    
    sprintf(buf, "mlx%d", unit);
    return(buf);
}

/* return path of drive (unit) */
char *
drivepath(int unit)
{
    static char	buf[32];
    
    sprintf(buf, "%s%s", _PATH_DEV, drivename(unit));
    return(buf);
}

/* return name of drive (unit) */
char *
drivename(int unit)
{
    static char	buf[32];
    
    sprintf(buf, "mlxd%d", unit);
    return(buf);
}

/* get controller unit number from name in (str) */
int
ctrlrunit(char *str)
{
    int		unit;
    
    if (sscanf(str, "mlx%d", &unit) == 1)
	return(unit);
    return(-1);
}

/* get drive unit number from name in (str) */
int
driveunit(char *str)
{
    int		unit;
    
    if (sscanf(str, "mlxd%d", &unit) == 1)
	return(unit);
    return(-1);
}

/********************************************************************************
 * Standardised output of various data structures.
 */

void
mlx_print_phys_drv(struct mlx_phys_drv *drv, int chn, int targ, char *prefix, int verbose)
{
    char	*type, *device, *vendor, *revision;

    switch(drv->pd_flags2 & 0x03) {
    case MLX_PHYS_DRV_DISK:
	type = "disk";
	break;
    case MLX_PHYS_DRV_SEQUENTIAL:
	type = "tape";
	break;
    case MLX_PHYS_DRV_CDROM:
	type= "cdrom";
	break;
    case MLX_PHYS_DRV_OTHER:
    default:
	type = "unknown";
	break;
    }
    printf("%s%s%02d%02d ", prefix, type, chn, targ);
    switch(drv->pd_status) {
    case MLX_PHYS_DRV_DEAD:
	printf(" (dead)       ");
	break;
    case MLX_PHYS_DRV_WRONLY:
	printf(" (write-only) ");
	break;
    case MLX_PHYS_DRV_ONLINE:
	printf(" (online)     ");
	break;
    case MLX_PHYS_DRV_STANDBY:
	printf(" (standby)    ");
	break;
    default:
	printf(" (0x%02x)   ", drv->pd_status);
    }
    printf("\n");
    
    if (verbose) {
	
	printf("%s   ", prefix);
	if (!mlx_scsi_inquiry(0, chn, targ, &vendor, &device, &revision)) {
	    printf("'%8.8s' '%16.16s' '%4.4s'", vendor, device, revision);
	} else {
	    printf("<IDENTIFY FAILED>");
	}
    
	printf(" %dMB ", drv->pd_config_size / 2048);
    
	if (drv->pd_flags2 & MLX_PHYS_DRV_FAST20) {
	    printf(" fast20");
	} else if (drv->pd_flags2 & MLX_PHYS_DRV_FAST) {
	    printf(" fast");
	}
	if (drv->pd_flags2 & MLX_PHYS_DRV_WIDE)
	    printf(" wide");
	if (drv->pd_flags2 & MLX_PHYS_DRV_SYNC)
	    printf(" sync");
	if (drv->pd_flags2 & MLX_PHYS_DRV_TAG)
	    printf(" tag-enabled");
	printf("\n");
    }
}