
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
			/*NOTREACHED */		    case 'v':
			verbose = 1;
			break;
		    default:
			warnx("invalid option %c", opt);
			help();
			/*NOTREACHED */
		}
	}
	if (optind >= argc)
		help();

	cp = argv[optind];
	cmd = lookupcmd(cp);
	if (cmd == NULL)
		errx(EX_DATAERR, "unknown command %s", cp);

	if ((cmd->flags & CMDFL_NO_KMOD) == 0 && smb_lib_init() != 0)
		exit(1);

	argc -= optind;
	argv += optind;
	optind = optreset = 1;
	return cmd->fn(argc, argv);
}

static void
help(void) {
	printf("\n");
	printf("usage: %s [-hv] command [args]\n", __progname);
	printf("where commands are:\n"
	" crypt [password]		slightly encrypt password\n"
	" help command			display help on \"command\"\n"
	" lc 				display active connections\n"
	" login //user@host[/share]	login to the specified host\n"
	" logout //user@host[/share]	logout from the specified host\n"
	" print //user@host/share file	print file to the specified remote printer\n"
	" view //user@host		list resources on the specified host\n"
	"\n");
	exit(1);
}

static void
help_usage(void) {
	printf("usage: smbutil help command\n");
	exit(1);
}