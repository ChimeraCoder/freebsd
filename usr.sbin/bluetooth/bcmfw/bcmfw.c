
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
#include <sys/ioctl.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <netgraph.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define BCMFW		"bcmfw"
#define BCMFW_INTR_EP	1
#define BCMFW_BULK_EP	2
#define BCMFW_BSIZE	4096

#define USB_VENDOR_BROADCOM 0x0a5c
#define USB_PRODUCT_BROADCOM_BCM2033 0x2033

static int bcmfw_check_device
	(char const *name);
static int bcmfw_load_firmware
	(char const *name, char const *md, char const *fw);
static void bcmfw_usage
	(void);

/*
 * Main
 */

int
main(int argc, char *argv[])
{
	char	*name = NULL, *md = NULL, *fw = NULL;
	int	 x;

	while ((x = getopt(argc, argv, "f:hn:m:")) != -1) {
		switch (x) {
		case 'f': /* firmware file */
			fw = optarg;
			break;

		case 'n': /* name */
			name = optarg;
			break;

		case 'm': /* Mini-driver */
			md = optarg;
			break;

		case 'h':
		default:
			bcmfw_usage();
			/* NOT REACHED */
		}
	}

	if (name == NULL || md == NULL || fw == NULL)
		bcmfw_usage();
		/* NOT REACHED */

	openlog(BCMFW, LOG_NDELAY|LOG_PERROR|LOG_PID, LOG_USER);

	if (bcmfw_check_device(name) < 0)
		exit(1);

	if (bcmfw_load_firmware(name, md, fw) < 0)
		exit(1);

	closelog();

	return (0);
} /* main */

/*
 * Check device VendorID/ProductID
 */

static int
bcmfw_check_device(char const *name)
{
	usb_device_descriptor_t	desc;
	char			path[BCMFW_BSIZE];
	int			fd = -1, error = -1;

	snprintf(path, sizeof(path), "/dev/%s", name);

	if ((fd = open(path, O_WRONLY)) < 0) {
		syslog(LOG_ERR, "Could not open(%s). %s (%d)",
				path, strerror(errno), errno);
		goto out;
	}

	if (ioctl(fd, USB_GET_DEVICE_DESC, &desc) < 0) {
		syslog(LOG_ERR, "Could not ioctl(%d, %ld, %p). %s (%d)",
				fd, USB_GET_DEVICE_DESC, &desc,
				strerror(errno), errno);
		goto out;
	}

	if (UGETW(desc.idVendor) != USB_VENDOR_BROADCOM ||
	    UGETW(desc.idProduct) != USB_PRODUCT_BROADCOM_BCM2033) {
		syslog(LOG_ERR, "Unsupported device, VendorID=%#x, " \
				"ProductID=%#x", UGETW(desc.idVendor),
				UGETW(desc.idProduct));
		error = -1;
	} else
		error = 0;
out:
	if (fd != -1)
		close(fd);

	return (error);
} /* bcmfw_check_device */

/*
 * Download minidriver and firmware
 */

static int
bcmfw_load_firmware(char const *name, char const *md, char const *fw)
{
	char	buf[BCMFW_BSIZE];
	int	intr = -1, bulk = -1, fd = -1, error = -1, len;

	/* Open interrupt endpoint device */
	snprintf(buf, sizeof(buf), "/dev/%s.%d", name, BCMFW_INTR_EP);
	if ((intr = open(buf, O_RDONLY)) < 0) {
		syslog(LOG_ERR, "Could not open(%s). %s (%d)",
				buf, strerror(errno), errno);
		goto out;
	}

	/* Open bulk endpoint device */
	snprintf(buf, sizeof(buf), "/dev/%s.%d", name, BCMFW_BULK_EP);
	if ((bulk = open(buf, O_WRONLY)) < 0) {
		syslog(LOG_ERR, "Could not open(%s). %s (%d)",
				buf, strerror(errno), errno);
		goto out;
	}

	/* 
	 * Load mini-driver 
	 */

	if ((fd = open(md, O_RDONLY)) < 0) {
		syslog(LOG_ERR, "Could not open(%s). %s (%d)",
				md, strerror(errno), errno);
		goto out;
	}

	for (;;) {
		len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			syslog(LOG_ERR, "Could not read(%s). %s (%d)",
					md, strerror(errno), errno);
			goto out;
		}
		if (len == 0)
			break;

		len = write(bulk, buf, len);
		if (len < 0) {
			syslog(LOG_ERR, "Could not write(/dev/%s.%d). %s (%d)",
					name, BCMFW_BULK_EP, strerror(errno),
					errno);
			goto out;
		}
	}

	close(fd);
	fd = -1;

	usleep(10);

	/* 
	 * Memory select 
	 */

	if (write(bulk, "#", 1) < 0) {
		syslog(LOG_ERR, "Could not write(/dev/%s.%d). %s (%d)",
				name, BCMFW_BULK_EP, strerror(errno), errno);
		goto out;
	}

	if (read(intr, buf, sizeof(buf)) < 0) {
		syslog(LOG_ERR, "Could not read(/dev/%s.%d). %s (%d)",
				name, BCMFW_INTR_EP, strerror(errno), errno);
		goto out;
	}

	if (buf[0] != '#') {
		syslog(LOG_ERR, "%s: Memory select failed (%c)", name, buf[0]);
		goto out;
	}

	/*
	 * Load firmware
	 */

	if ((fd = open(fw, O_RDONLY)) < 0) {
		syslog(LOG_ERR, "Could not open(%s). %s (%d)",
				fw, strerror(errno), errno);
		goto out;
	}

	for (;;) {
		len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			syslog(LOG_ERR, "Could not read(%s). %s (%d)",
					fw, strerror(errno), errno);
			goto out;
		}
		if (len == 0)
			break;

		len = write(bulk, buf, len);
		if (len < 0) {
			syslog(LOG_ERR, "Could not write(/dev/%s.%d). %s (%d)",
					name, BCMFW_BULK_EP, strerror(errno),
					errno);
			goto out;
		}
	}

	close(fd);
	fd = -1;

	if (read(intr, buf, sizeof(buf)) < 0) {
		syslog(LOG_ERR, "Could not read(/dev/%s.%d). %s (%d)",
				name, BCMFW_INTR_EP, strerror(errno), errno);
		goto out;
	}

	if (buf[0] != '.') {
		syslog(LOG_ERR, "%s: Could not load firmware (%c)",
				name, buf[0]);
		goto out;
	}

	usleep(500000);
	error = 0;
out:
	if (fd != -1)
		close(fd);
	if (bulk != -1)
		close(bulk);
	if (intr != -1)
		close(intr);

	return (error);
} /* bcmfw_load_firmware */

/*
 * Display usage message and quit
 */

static void 
bcmfw_usage(void)
{
	fprintf(stdout,
"Usage: %s -n name -m md_file -f fw_file\n"
"Where:\n" \
"\t-n name              device name\n" \
"\t-m mini-driver       image mini-driver image file name for download\n" \
"\t-f firmware image    firmware image file name for download\n" \
"\t-h                   display this message\n", BCMFW);

	exit(255);
} /* bcmfw_usage */