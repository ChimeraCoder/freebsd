
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

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_media.h>
#include <dev/etherswitch/etherswitch.h>

int	get_media_subtype(int, const char *);
int	get_media_mode(int, const char *);
int	get_media_options(int, const char *);
int	lookup_media_word(struct ifmedia_description *, const char *);
void    print_media_word(int, int);
void    print_media_word_ifconfig(int);

/* some constants */
#define IEEE802DOT1Q_VID_MAX	4094
#define IFMEDIAREQ_NULISTENTRIES	256

enum cmdmode {
	MODE_NONE = 0,
	MODE_PORT,
	MODE_VLANGROUP,
	MODE_REGISTER,
	MODE_PHYREG
};

struct cfg {
	int					fd;
	int					verbose;
	int					mediatypes;
	const char			*controlfile;
	etherswitch_info_t	info;
	enum cmdmode		mode;
	int					unit;
};

struct cmds {
	enum cmdmode	mode;
	const char		*name;
	int				args;
	void 			(*f)(struct cfg *, char *argv[]);
};
static struct cmds cmds[];


static void usage(void);

static int
read_register(struct cfg *cfg, int r)
{
	struct etherswitch_reg er;
	
	er.reg = r;
	if (ioctl(cfg->fd, IOETHERSWITCHGETREG, &er) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETREG)");
	return (er.val);
}

static void
write_register(struct cfg *cfg, int r, int v)
{
	struct etherswitch_reg er;
	
	er.reg = r;
	er.val = v;
	if (ioctl(cfg->fd, IOETHERSWITCHSETREG, &er) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHSETREG)");
}

static int
read_phyregister(struct cfg *cfg, int phy, int reg)
{
	struct etherswitch_phyreg er;
	
	er.phy = phy;
	er.reg = reg;
	if (ioctl(cfg->fd, IOETHERSWITCHGETPHYREG, &er) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETPHYREG)");
	return (er.val);
}

static void
write_phyregister(struct cfg *cfg, int phy, int reg, int val)
{
	struct etherswitch_phyreg er;
	
	er.phy = phy;
	er.reg = reg;
	er.val = val;
	if (ioctl(cfg->fd, IOETHERSWITCHSETPHYREG, &er) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHSETPHYREG)");
}

static void
set_port_vid(struct cfg *cfg, char *argv[])
{
	int v;
	etherswitch_port_t p;
	
	v = strtol(argv[1], NULL, 0);
	if (v < 0 || v > IEEE802DOT1Q_VID_MAX)
		errx(EX_USAGE, "pvid must be between 0 and %d",
		    IEEE802DOT1Q_VID_MAX);
	bzero(&p, sizeof(p));
	p.es_port = cfg->unit;
	if (ioctl(cfg->fd, IOETHERSWITCHGETPORT, &p) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETPORT)");
	p.es_pvid = v;
	if (ioctl(cfg->fd, IOETHERSWITCHSETPORT, &p) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHSETPORT)");
}

static void
set_port_media(struct cfg *cfg, char *argv[])
{
	etherswitch_port_t p;
	int ifm_ulist[IFMEDIAREQ_NULISTENTRIES];
	int subtype;
	
	bzero(&p, sizeof(p));
	p.es_port = cfg->unit;
	p.es_ifmr.ifm_ulist = ifm_ulist;
	p.es_ifmr.ifm_count = IFMEDIAREQ_NULISTENTRIES;
	if (ioctl(cfg->fd, IOETHERSWITCHGETPORT, &p) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETPORT)");
	subtype = get_media_subtype(IFM_TYPE(ifm_ulist[0]), argv[1]);
	p.es_ifr.ifr_media = (p.es_ifmr.ifm_current & IFM_IMASK) |
	        IFM_TYPE(ifm_ulist[0]) | subtype;
	if (ioctl(cfg->fd, IOETHERSWITCHSETPORT, &p) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHSETPORT)");
}

static void
set_port_mediaopt(struct cfg *cfg, char *argv[])
{
	etherswitch_port_t p;
	int ifm_ulist[IFMEDIAREQ_NULISTENTRIES];
	int options;
	
	bzero(&p, sizeof(p));
	p.es_port = cfg->unit;
	p.es_ifmr.ifm_ulist = ifm_ulist;
	p.es_ifmr.ifm_count = IFMEDIAREQ_NULISTENTRIES;
	if (ioctl(cfg->fd, IOETHERSWITCHGETPORT, &p) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETPORT)");
	options = get_media_options(IFM_TYPE(ifm_ulist[0]), argv[1]);
	if (options == -1)
		errx(EX_USAGE, "invalid media options \"%s\"", argv[1]);
	if (options & IFM_HDX) {
		p.es_ifr.ifr_media &= ~IFM_FDX;
		options &= ~IFM_HDX;
	}
	p.es_ifr.ifr_media |= options;
	if (ioctl(cfg->fd, IOETHERSWITCHSETPORT, &p) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHSETPORT)");
}

static void
set_vlangroup_vid(struct cfg *cfg, char *argv[])
{
	int v;
	etherswitch_vlangroup_t vg;
	
	v = strtol(argv[1], NULL, 0);
	if (v < 0 || v >= IEEE802DOT1Q_VID_MAX)
		errx(EX_USAGE, "vlan must be between 0 and %d", IEEE802DOT1Q_VID_MAX);
	vg.es_vlangroup = cfg->unit;
	if (ioctl(cfg->fd, IOETHERSWITCHGETVLANGROUP, &vg) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETVLANGROUP)");
	vg.es_vid = v;
	if (ioctl(cfg->fd, IOETHERSWITCHSETVLANGROUP, &vg) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHSETVLANGROUP)");
}

static void
set_vlangroup_members(struct cfg *cfg, char *argv[])
{
	etherswitch_vlangroup_t vg;
	int member, untagged;
	char *c, *d;
	int v;
	
	member = untagged = 0;
	if (strcmp(argv[1], "none") != 0) {
		for (c=argv[1]; *c; c=d) {
			v = strtol(c, &d, 0);
			if (d == c)
				break;
			if (v < 0 || v >= cfg->info.es_nports)
				errx(EX_USAGE, "Member port must be between 0 and %d", cfg->info.es_nports-1);
			if (d[0] == ',' || d[0] == '\0' ||
				((d[0] == 't' || d[0] == 'T') && (d[1] == ',' || d[1] == '\0'))) {
				if (d[0] == 't' || d[0] == 'T') {
					untagged &= ~ETHERSWITCH_PORTMASK(v);
					d++;
				} else
					untagged |= ETHERSWITCH_PORTMASK(v);
				member |= ETHERSWITCH_PORTMASK(v);
				d++;
			} else
				errx(EX_USAGE, "Invalid members specification \"%s\"", d);
		}
	}
	vg.es_vlangroup = cfg->unit;
	if (ioctl(cfg->fd, IOETHERSWITCHGETVLANGROUP, &vg) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETVLANGROUP)");
	vg.es_member_ports = member;
	vg.es_untagged_ports = untagged;
	if (ioctl(cfg->fd, IOETHERSWITCHSETVLANGROUP, &vg) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHSETVLANGROUP)");
}

static int
set_register(struct cfg *cfg, char *arg)
{
	int a, v;
	char *c;
	
	a = strtol(arg, &c, 0);
	if (c==arg)
		return (1);
	if (*c == '=') {
		v = strtol(c+1, NULL, 0);
		write_register(cfg, a, v);
	}
	printf("\treg 0x%04x=0x%04x\n", a, read_register(cfg, a));
	return (0);
}

static int
set_phyregister(struct cfg *cfg, char *arg)
{
	int phy, reg, val;
	char *c, *d;
	
	phy = strtol(arg, &c, 0);
	if (c==arg)
		return (1);
	if (*c != '.')
		return (1);
	d = c+1;
	reg = strtol(d, &c, 0);
	if (d == c)
		return (1);
	if (*c == '=') {
		val = strtol(c+1, NULL, 0);
		write_phyregister(cfg, phy, reg, val);
	}
	printf("\treg %d.0x%02x=0x%04x\n", phy, reg, read_phyregister(cfg, phy, reg));
	return (0);
}

static void
print_port(struct cfg *cfg, int port)
{
	etherswitch_port_t p;
	int ifm_ulist[IFMEDIAREQ_NULISTENTRIES];
	int i;

	bzero(&p, sizeof(p));
	p.es_port = port;
	p.es_ifmr.ifm_ulist = ifm_ulist;
	p.es_ifmr.ifm_count = IFMEDIAREQ_NULISTENTRIES;
	if (ioctl(cfg->fd, IOETHERSWITCHGETPORT, &p) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETPORT)");
	printf("port%d:\n", port);
	printf("\tpvid: %d\n", p.es_pvid);
	printf("\tmedia: ");
	print_media_word(p.es_ifmr.ifm_current, 1);
	if (p.es_ifmr.ifm_active != p.es_ifmr.ifm_current) {
		putchar(' ');
		putchar('(');
		print_media_word(p.es_ifmr.ifm_active, 0);
		putchar(')');
	}
	putchar('\n');
	printf("\tstatus: %s\n", (p.es_ifmr.ifm_status & IFM_ACTIVE) != 0 ? "active" : "no carrier");
	if (cfg->mediatypes) {
		printf("\tsupported media:\n");
		if (p.es_ifmr.ifm_count > IFMEDIAREQ_NULISTENTRIES)
			p.es_ifmr.ifm_count = IFMEDIAREQ_NULISTENTRIES;
		for (i=0; i<p.es_ifmr.ifm_count; i++) {
			printf("\t\tmedia ");
			print_media_word(ifm_ulist[i], 0);
			putchar('\n');
		}
	}
}

static void
print_vlangroup(struct cfg *cfg, int vlangroup)
{
	etherswitch_vlangroup_t vg;
	int i, comma;
	
	vg.es_vlangroup = vlangroup;
	if (ioctl(cfg->fd, IOETHERSWITCHGETVLANGROUP, &vg) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETVLANGROUP)");
	if (cfg->verbose == 0 && vg.es_member_ports == 0)
		return;
	printf("vlangroup%d:\n", vlangroup);
	printf("\tvlan: %d\n", vg.es_vid);
	printf("\tmembers ");
	comma = 0;
	if (vg.es_member_ports != 0)
		for (i=0; i<cfg->info.es_nports; i++) {
			if ((vg.es_member_ports & ETHERSWITCH_PORTMASK(i)) != 0) {
				if (comma)
					printf(",");
				printf("%d", i);
				if ((vg.es_untagged_ports & ETHERSWITCH_PORTMASK(i)) == 0)
					printf("t");
				comma = 1;
			}
		}
	else
		printf("none");
	printf("\n");
}

static void
print_info(struct cfg *cfg)
{
	const char *c;
	int i;
	
	c = strrchr(cfg->controlfile, '/');
	if (c != NULL)
		c = c + 1;
	else
		c = cfg->controlfile;
	if (cfg->verbose)
		printf("%s: %s with %d ports and %d VLAN groups\n",
			c, cfg->info.es_name, cfg->info.es_nports, cfg->info.es_nvlangroups);
	for (i=0; i<cfg->info.es_nports; i++) {
		print_port(cfg, i);
	}
	for (i=0; i<cfg->info.es_nvlangroups; i++) {
		print_vlangroup(cfg, i);
	}
}

static void
usage(void)
{
	fprintf(stderr, "usage: etherswitchctl\n");
	exit(EX_USAGE);
}

static void
newmode(struct cfg *cfg, enum cmdmode mode)
{
	if (mode == cfg->mode)
		return;
	switch (cfg->mode) {
	case MODE_NONE:
		break;
	case MODE_PORT:
		print_port(cfg, cfg->unit);
		break;
	case MODE_VLANGROUP:
		print_vlangroup(cfg, cfg->unit);
		break;
	case MODE_REGISTER:
	case MODE_PHYREG:
		break;
	}
	cfg->mode = mode;
}

int
main(int argc, char *argv[])
{
	int ch;
	struct cfg cfg;
	int i;
	
	bzero(&cfg, sizeof(cfg));
	cfg.controlfile = "/dev/etherswitch0";
	while ((ch = getopt(argc, argv, "f:mv?")) != -1)
		switch(ch) {
		case 'f':
			cfg.controlfile = optarg;
			break;
		case 'm':
			cfg.mediatypes++;
			break;
		case 'v':
			cfg.verbose++;
			break;
		case '?':
			/* FALLTHROUGH */
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	cfg.fd = open(cfg.controlfile, O_RDONLY);
	if (cfg.fd < 0)
		err(EX_UNAVAILABLE, "Can't open control file: %s", cfg.controlfile);
	if (ioctl(cfg.fd, IOETHERSWITCHGETINFO, &cfg.info) != 0)
		err(EX_OSERR, "ioctl(IOETHERSWITCHGETINFO)");
	if (argc == 0) {
		print_info(&cfg);
		return (0);
	}
	cfg.mode = MODE_NONE;
	while (argc > 0) {
		switch(cfg.mode) {
		case MODE_NONE:
			if (strcmp(argv[0], "info") == 0) {
				print_info(&cfg);
			} else if (sscanf(argv[0], "port%d", &cfg.unit) == 1) {
				if (cfg.unit < 0 || cfg.unit >= cfg.info.es_nports)
					errx(EX_USAGE, "port unit must be between 0 and %d", cfg.info.es_nports);
				newmode(&cfg, MODE_PORT);
			} else if (sscanf(argv[0], "vlangroup%d", &cfg.unit) == 1) {
				if (cfg.unit < 0 || cfg.unit >= cfg.info.es_nvlangroups)
					errx(EX_USAGE, "port unit must be between 0 and %d", cfg.info.es_nvlangroups);
				newmode(&cfg, MODE_VLANGROUP);
			} else if (strcmp(argv[0], "phy") == 0) {
				newmode(&cfg, MODE_PHYREG);
			} else if (strcmp(argv[0], "reg") == 0) {
				newmode(&cfg, MODE_REGISTER);
			} else {
				errx(EX_USAGE, "Unknown command \"%s\"", argv[0]);
			}
			break;
		case MODE_PORT:
		case MODE_VLANGROUP:
			for(i=0; cmds[i].name != NULL; i++) {
				if (cfg.mode == cmds[i].mode && strcmp(argv[0], cmds[i].name) == 0) {
					if (argc < (cmds[i].args + 1)) {
						printf("%s needs an argument\n", cmds[i].name);
						break;
					}
					(cmds[i].f)(&cfg, argv);
					argc -= cmds[i].args;
					argv += cmds[i].args;
					break;
				}
			}
			if (cmds[i].name == NULL) {
				newmode(&cfg, MODE_NONE);
				continue;
			}
			break;
		case MODE_REGISTER:
			if (set_register(&cfg, argv[0]) != 0) {
				newmode(&cfg, MODE_NONE);
				continue;
			}
			break;
		case MODE_PHYREG:
			if (set_phyregister(&cfg, argv[0]) != 0) {
				newmode(&cfg, MODE_NONE);
				continue;
			}
			break;
		}
		argc--;
		argv++;
	}
	/* switch back to command mode to print configuration for last command */
	newmode(&cfg, MODE_NONE);
	close(cfg.fd);
	return (0);
}

static struct cmds cmds[] = {
	{ MODE_PORT, "pvid", 1, set_port_vid },
	{ MODE_PORT, "media", 1, set_port_media },
	{ MODE_PORT, "mediaopt", 1, set_port_mediaopt },
	{ MODE_VLANGROUP, "vlan", 1, set_vlangroup_vid },
	{ MODE_VLANGROUP, "members", 1, set_vlangroup_members },
	{ 0, NULL, 0, NULL }
};