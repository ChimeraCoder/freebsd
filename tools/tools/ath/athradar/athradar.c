
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

#include "diag.h"

#include "ah.h"
#include "ah_internal.h"

#ifndef	ATH_DEFAULT
#define	ATH_DEFAULT	"ath0"
#endif

#include <getopt.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

struct radarhandler {
	struct		ath_diag atd;
	int		s;
	struct ifreq	ifr;
	int		ah_devid;
};

int
radar_opendev(struct radarhandler *radar, const char *devid)
{
	HAL_REVS revs;

	radar->s = socket(AF_INET, SOCK_DGRAM, 0);
	if (radar->s < 0) {
		warn("socket");
		return 0;
	}

	strncpy(radar->atd.ad_name, devid, sizeof (radar->atd.ad_name));

	/* Get the hardware revision, just to verify things are working */
	radar->atd.ad_id = HAL_DIAG_REVS;
	radar->atd.ad_out_data = (caddr_t) &revs;
	radar->atd.ad_out_size = sizeof(revs);
	if (ioctl(radar->s, SIOCGATHDIAG, &radar->atd) < 0) {
		warn(radar->atd.ad_name);
		return 0;
	}
	radar->ah_devid = revs.ah_devid;
	return 1;
}

void
radar_closedev(struct radarhandler *radar)
{
	close(radar->s);
	radar->s = -1;
}

void
radarset(struct radarhandler *radar, int op, u_int32_t param)
{
	HAL_PHYERR_PARAM pe;

	pe.pe_firpwr = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_rrssi = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_height = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_prssi = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_inband = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_enabled = HAL_PHYERR_PARAM_NOVAL;

	pe.pe_relpwr = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_relstep = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_maxlen = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_usefir128 = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_blockradar = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_enmaxrssi = HAL_PHYERR_PARAM_NOVAL;

	pe.pe_extchannel = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_enrelpwr = HAL_PHYERR_PARAM_NOVAL;
	pe.pe_en_relstep_check = HAL_PHYERR_PARAM_NOVAL;

	switch (op) {
	case DFS_PARAM_ENABLE:
		pe.pe_enabled = param;
		break;
	case DFS_PARAM_FIRPWR:
		pe.pe_firpwr = param;
		break;
	case DFS_PARAM_RRSSI:
		pe.pe_rrssi = param;
		break;
	case DFS_PARAM_HEIGHT:
		pe.pe_height = param;
		break;
	case DFS_PARAM_PRSSI:
		pe.pe_prssi = param;
		break;
	case DFS_PARAM_INBAND:
		pe.pe_inband = param;
		break;
	case DFS_PARAM_RELPWR:
		pe.pe_relpwr = param;
		break;
	case DFS_PARAM_RELSTEP:
		pe.pe_relstep = param;
		break;
	case DFS_PARAM_MAXLEN:
		pe.pe_maxlen = param;
		break;
	case DFS_PARAM_USEFIR128:
		pe.pe_usefir128 = param;
		break;
	case DFS_PARAM_BLOCKRADAR:
		pe.pe_blockradar = param;
		break;
	case DFS_PARAM_MAXRSSI_EN:
		pe.pe_enmaxrssi = param;
		break;
	case DFS_PARAM_EN_EXTCH:
		pe.pe_extchannel = param;
		break;
	case DFS_PARAM_RELPWR_EN:
		pe.pe_enrelpwr = param;
		break;
	case DFS_PARAM_RELSTEP_EN:
		pe.pe_en_relstep_check = param;
		break;
	}

	radar->atd.ad_id = DFS_SET_THRESH | ATH_DIAG_IN;
	radar->atd.ad_out_data = NULL;
	radar->atd.ad_out_size = 0;
	radar->atd.ad_in_data = (caddr_t) &pe;
	radar->atd.ad_in_size = sizeof(HAL_PHYERR_PARAM);
	if (ioctl(radar->s, SIOCGATHPHYERR, &radar->atd) < 0)
		err(1, radar->atd.ad_name);
}

static void
radar_get(struct radarhandler *radar)
{
	HAL_PHYERR_PARAM pe;

	radar->atd.ad_id = DFS_GET_THRESH | ATH_DIAG_DYN;
	memset(&pe, 0, sizeof(pe));

	radar->atd.ad_in_data = NULL;
	radar->atd.ad_in_size = 0;
	radar->atd.ad_out_data = (caddr_t) &pe;
	radar->atd.ad_out_size = sizeof(pe);

	if (ioctl(radar->s, SIOCGATHPHYERR, &radar->atd) < 0)
		err(1, radar->atd.ad_name);

	printf("Radar parameters (raw):\n");
	printf("    pe_enabled: %d\n", pe.pe_enabled);
	printf("    pe_firpwr: %d\n", pe.pe_firpwr);
	printf("    pe_rrssi: %d\n", pe.pe_rrssi);
	printf("    pe_height: %d\n", pe.pe_height);
	printf("    pe_prssi: %d\n", pe.pe_prssi);
	printf("    pe_inband: %d\n", pe.pe_inband);
	printf("    pe_relpwr: %d\n", pe.pe_relpwr);
	printf("    pe_relstep: %d\n", pe.pe_relstep);
	printf("    pe_maxlen: %d\n", pe.pe_maxlen);
	printf("    pe_usefir128: %d\n", pe.pe_usefir128);
	printf("    pe_blockradar: %d\n", pe.pe_blockradar);
	printf("    pe_enmaxrssi: %d\n", pe.pe_enmaxrssi);
	printf("    pe_extchannel: %d\n", pe.pe_extchannel);
	printf("    pe_enrelpwr: %d\n", pe.pe_enrelpwr);
	printf("    pe_en_relstep_check: %d\n", pe.pe_en_relstep_check);
}

static int
radar_set_param(struct radarhandler *radar, const char *param,
    const char *val)
{
	int v;

	v = atoi(val);

	if (strcmp(param, "enabled") == 0) {
		radarset(radar, DFS_PARAM_ENABLE, v);
	} else if (strcmp(param, "firpwr") == 0) {
		radarset(radar, DFS_PARAM_FIRPWR, v);
	} else if (strcmp(param, "rrssi") == 0) {
		radarset(radar, DFS_PARAM_RRSSI, v);
	} else if (strcmp(param, "height") == 0) {
		radarset(radar, DFS_PARAM_HEIGHT, v);
	} else if (strcmp(param, "prssi") == 0) {
		radarset(radar, DFS_PARAM_PRSSI, v);
	} else if (strcmp(param, "inband") == 0) {
		radarset(radar, DFS_PARAM_INBAND, v);
	} else if (strcmp(param, "relpwr") == 0) {
		radarset(radar, DFS_PARAM_RELPWR, v);
	} else if (strcmp(param, "relstep") == 0) {
		radarset(radar, DFS_PARAM_RELSTEP, v);
	} else if (strcmp(param, "maxlen") == 0) {
		radarset(radar, DFS_PARAM_MAXLEN, v);
	} else if (strcmp(param, "usefir128") == 0) {
		radarset(radar, DFS_PARAM_USEFIR128, v);
	} else if (strcmp(param, "blockradar") == 0) {
		radarset(radar, DFS_PARAM_BLOCKRADAR, v);
	} else if (strcmp(param, "enmaxrssi") == 0) {
		radarset(radar, DFS_PARAM_MAXRSSI_EN, v);
	} else if (strcmp(param, "extchannel") == 0) {
		radarset(radar, DFS_PARAM_EN_EXTCH, v);
	} else if (strcmp(param, "enrelpwr") == 0) {
		radarset(radar, DFS_PARAM_RELPWR_EN, v);
	} else if (strcmp(param, "en_relstep_check") == 0) {
		radarset(radar, DFS_PARAM_RELSTEP_EN, v);
	} else {
		return 0;
	}

	return 1;
}

void
usage(const char *progname)
{
	printf("Usage:\n");
	printf("\t%s: [-i <interface>] <cmd> (<arg>)\n", progname);
	printf("\t%s: [-h]\n", progname);
	printf("\n");
	printf("Valid commands:\n");
	printf("\tget:\t\tGet current radar parameters\n");
	printf("\tset <param> <value>:\t\tSet radar parameter\n");
}

int
main(int argc, char *argv[])
{
	struct radarhandler radar;
	const char *devname = ATH_DEFAULT;
	const char *progname = argv[0];

	memset(&radar, 0, sizeof(radar));

	/* Parse command line options */
	if (argc >= 2 && strcmp(argv[1], "-h") == 0) {
		usage(progname);
		exit(0);
	}
	if (argc >= 2 && strcmp(argv[1], "-?") == 0) {
		usage(progname);
		exit(0);
	}

	if (argc >= 2 && strcmp(argv[1], "-i") == 0) {
		if (argc == 2) {
			usage(progname);
			exit(127);
		}
		devname = argv[2];
		argc -= 2; argv += 2;
	}

	/* At this point we require at least one command */
	if (argc == 1) {
		usage(progname);
		exit(127);
	}

	if (radar_opendev(&radar, devname) == 0)
		exit(127);

	if (strcasecmp(argv[1], "get") == 0) {
		radar_get(&radar);
	} else if (strcasecmp(argv[1], "set") == 0) {
		if (argc < 4) {
			usage(progname);
			exit(127);
		}
		if (radar_set_param(&radar, argv[2], argv[3]) == 0) {
			usage(progname);
			exit(127);
		}
	} else {
		usage(progname);
		exit(127);
	}

	/* wrap up */
	radar_closedev(&radar);
	exit(0);
}