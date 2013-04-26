
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

#include "qla_os.h"
#include "qla_reg.h"
#include "qla_hw.h"
#include "qla_def.h"
#include "qla_reg.h"
#include "qla_inline.h"
#include "qla_glbl.h"
#include "qla_ioctl.h"

static struct cdevsw qla_cdevsw = {
	.d_version = D_VERSION,
	.d_ioctl = qla_eioctl,
	.d_name = "qlcnic",
};

int
qla_make_cdev(qla_host_t *ha)
{
        ha->ioctl_dev = make_dev(&qla_cdevsw,
				ha->ifp->if_dunit,
                                UID_ROOT,
                                GID_WHEEL,
                                0600,
                                "%s",
                                if_name(ha->ifp));

	if (ha->ioctl_dev == NULL)
		return (-1);

        ha->ioctl_dev->si_drv1 = ha;

	return (0);
}

void
qla_del_cdev(qla_host_t *ha)
{
	if (ha->ioctl_dev != NULL)
		destroy_dev(ha->ioctl_dev);
	return;
}

int
qla_eioctl(struct cdev *dev, u_long cmd, caddr_t data, int fflag,
        struct thread *td)
{
        qla_host_t *ha;
        int rval = 0;
        qla_reg_val_t *rv;
        qla_rd_flash_t *rdf;

        if ((ha = (qla_host_t *)dev->si_drv1) == NULL)
                return ENXIO;

        switch(cmd) {

        case QLA_RDWR_REG:

                rv = (qla_reg_val_t *)data;

                if (rv->direct) {
                        if (rv->rd) {
                                rv->val = READ_OFFSET32(ha, rv->reg);
                        } else {
                                WRITE_OFFSET32(ha, rv->reg, rv->val);
                        }
                } else {
                        if ((rval = qla_rdwr_indreg32(ha, rv->reg, &rv->val,
                                rv->rd)))
                                rval = ENXIO;
                }
                break;

        case QLA_RD_FLASH:
                rdf = (qla_rd_flash_t *)data;
                if ((rval = qla_rd_flash32(ha, rdf->off, &rdf->data)))
                        rval = ENXIO;
                break;
        default:
                break;
        }

        return rval;
}