
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

#include "xge_log.h"

/**
 * xge_print_hwstats
 * Prints/logs hardware statistics
 *
 * @hw_stats Hardware statistics
 * @device_id Device ID
 */
void
xge_print_hwstats(void *hw_stats, unsigned short device_id)
{
	int index = 0, count = 0;

	count = XGE_COUNT_STATS -
	    ((device_id == DEVICE_ID_XFRAME_II) ? 0 : XGE_COUNT_EXTENDED_STATS);

	fdAll = fopen("stats.log", "w+");
	if(!fdAll)
	    goto _exit;

	XGE_PRINT_HEADER_STATS(fdAll);
	for(index = 0; index < count ; index++) {
	    switch(statsInfo[index].type) {
	        case 2:
	            statsInfo[index].value =
	                *((u16 *)((unsigned char *)hw_stats +
	                GET_OFFSET_STATS(index)));
	            break;
	        case 4:
	            statsInfo[index].value =
	                *((u32 *)((unsigned char *) hw_stats +
	                GET_OFFSET_STATS(index)));
	            break;
	        case 8:
	            statsInfo[index].value =
	                *((u64 *)((unsigned char *)hw_stats +
	                GET_OFFSET_STATS(index)));
	            break;
	    }

	    XGE_PRINT_STATS(fdAll,(const char *) statsInfo[index].name,
	        statsInfo[index].value);
	}
	XGE_PRINT_LINE(fdAll);
	fclose(fdAll);
_exit:
	return;
}

/**
 * xge_print_pciconf
 * Prints/logs PCI configuration space
 *
 * @pci_conf PCI Configuration
 */
void
xge_print_pciconf(void * pci_conf)
{
	int index = 0;

	fdAll = fopen("pciconf.log", "w+");
	if(!fdAll)
	    goto _exit;

	XGE_PRINT_HEADER_PCICONF(fdAll);
	for(index = 0; index < XGE_COUNT_PCICONF; index++) {
	    pciconfInfo[index].value = *((u16 *)((unsigned char *)pci_conf +
	        GET_OFFSET_PCICONF(index)));
	    XGE_PRINT_PCICONF(fdAll,(const char *) pciconfInfo[index].name,
	        GET_OFFSET_PCICONF(index), pciconfInfo[index].value);
	}

	XGE_PRINT_LINE(fdAll);
	fclose(fdAll);

_exit:
	return;
}

/**
 * xge_print_devconf
 * Prints/logs Device Configuration
 *
 * @dev_conf Device Configuration
 */
void
xge_print_devconf(void * dev_conf)
{
	int index = 0;

	fdAll = fopen("devconf.log", "w+");
	if(!fdAll)
	    goto _exit;

	XGE_PRINT_HEADER_DEVCONF(fdAll);

	for(index = 0; index < XGE_COUNT_DEVCONF; index++) {
	    devconfInfo[index].value = *((u32 *)((unsigned char *)dev_conf +
	        (index * (sizeof(int)))));
	    XGE_PRINT_DEVCONF(fdAll,(const char *) devconfInfo[index].name,
	        devconfInfo[index].value);
	}

	XGE_PRINT_LINE(fdAll);
	fclose( fdAll );

_exit:
	return;
}

/**
 * xge_print_registers
 * Prints/logs Register values
 *
 * @registers Register values
 */
void
xge_print_registers(void * registers)
{
	int index = 0;

	fdAll = fopen("reginfo.log", "w+");
	if(!fdAll)
	    goto _exit;

	XGE_PRINT_HEADER_REGS(fdAll);

	for(index = 0; index < XGE_COUNT_REGS; index++) {
	    regInfo[index].value = *((u64 *)((unsigned char *)registers +
	        regInfo[index].offset));
	    XGE_PRINT_REGS(fdAll,(const char *) regInfo[index].name,
	        regInfo[index].offset, regInfo[index].value);
	}

	XGE_PRINT_LINE(fdAll);
	fclose(fdAll);
_exit:
	return;
}

/**
 * xge_print_register
 * Prints/logs a register value
 *
 * @offset Offset of the register
 * @temp???
 */
void
xge_print_register(u64 offset, u64 value)
{
	int index = 0;

	fdAll = fopen("readreg.log", "w+");
	if(!fdAll)
	    goto _exit;

	XGE_PRINT_READ_HEADER_REGS(fdAll);
	regInfo[index].offset = offset;
	regInfo[index].value  = value;
	printf("0x%.8X\t0x%.16llX\n", regInfo[index].offset,
	    regInfo[index].value);
	XGE_PRINT_LINE(fdAll);
	fclose(fdAll);

_exit:
	return;
}

/**
 * xge_print_devstats
 * Prints Device Statistics
 *
 * @dev_stats Device Statistics
 */
void
xge_print_devstats(void *dev_stats)
{
	int index = 0;

	fdAll = fopen("intrstats.log", "w+");
	if(!fdAll)
	    goto _exit;

	XGE_PRINT_HEADER_STATS(fdAll);
	for(index = 0; index < XGE_COUNT_INTRSTAT; index++) {
	    intrInfo[index].value = *((u32 *)((unsigned char *)dev_stats +
	        (index * (sizeof(u32)))));
	    XGE_PRINT_STATS(fdAll,(const char *) intrInfo[index].name,
	        intrInfo[index].value);
	}

	XGE_PRINT_LINE(fdAll);
	fclose(fdAll);
_exit:
	return;
}

/**
 * xge_print_swstats
 * Prints/logs Software Statistics
 *
 * @sw_stats Software statistics
 */
void
xge_print_swstats(void * sw_stats)
{
	int index = 0;

	fdAll = fopen("tcodestats.log", "w+");
	if(!fdAll)
	    goto _exit;

	XGE_PRINT_HEADER_STATS(fdAll);
	for(index = 0; index < XGE_COUNT_SWSTAT; index++) {
	    if(!(tcodeInfo[index].flag)) {
	        switch(tcodeInfo[index].type) {
	            case 2:
	                tcodeInfo[index].value =
	                    *((u16 *)((unsigned char *)sw_stats +
	                    (index * (sizeof(u16)))));
	                break;
	            case 4:
	                tcodeInfo[index].value =
	                    *((u32 *)((unsigned char *)sw_stats +
	                    (index * (sizeof(u32)))));
	                break;
	        }
	        XGE_PRINT_STATS(fdAll,(const char *) tcodeInfo[index].name,
	            tcodeInfo[index].value);
	    }
	}

	XGE_PRINT_LINE(fdAll);
	fclose(fdAll);

_exit:
	return;
}

/**
 * xge_print_drv_version
 * Prints/logs driver version
 *
 * @version Driver version
 */
void
xge_print_drv_version(char *version)
{
	fdAll = fopen("driverinfo.log", "w+");
	if(!fdAll)
	    goto _exit;

	XGE_PRINT_LINE(fdAll);
	printf("Driver Version: %s\n", version);
	XGE_PRINT_LINE(fdAll);
	fclose(fdAll);

_exit:
	return;
}

/**
 * xge_print_drvstats
 * Prints/logs Driver Statistics
 *
 * @driver_stats Driver Statistics
 */
void
xge_print_drvstats(void * driver_stats)
{
	int index = 0;

	fdAll = fopen("driver_stats.log", "w+");
	if(!fdAll)
	    goto _exit;

	XGE_PRINT_HEADER_STATS(fdAll);

	for(index = 0; index < XGE_COUNT_DRIVERSTATS; index++) {
	    driverInfo[index].value = *((u64 *)((unsigned char *)driver_stats +
	        (index * (sizeof(u64)))));
	    XGE_PRINT_STATS(fdAll,(const char *) driverInfo[index].name,
	        driverInfo[index].value);
	}

	XGE_PRINT_LINE(fdAll);
	fclose( fdAll );

_exit:
	return;
}