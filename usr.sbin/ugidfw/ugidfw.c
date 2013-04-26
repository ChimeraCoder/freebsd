
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

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <sys/sysctl.h>

#include <security/mac_bsdextended/mac_bsdextended.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ugidfw.h>

void add_rule(int argc, char *argv[]);
void list_rules(void);
void remove_rule(int argc, char *argv[]);
void set_rule(int argc, char *argv[]);
void usage(void);

void
usage(void)
{

	fprintf(stderr, "usage: ugidfw add [subject [not] [uid uid] [gid gid]]"
	    " [object [not] [uid uid] \\\n");
	fprintf(stderr, "    [gid gid]] mode arswxn\n");
	fprintf(stderr, "       ugidfw list\n");
	fprintf(stderr, "       ugidfw set rulenum [subject [not] [uid uid] [gid gid]]"
	    " [object [not] \\\n");
	fprintf(stderr, "    [uid uid] [gid gid]] mode arswxn\n");
	fprintf(stderr, "       ugidfw remove rulenum\n");

	exit(1);
}

void
add_rule(int argc, char *argv[])
{
	char errstr[BUFSIZ], charstr[BUFSIZ];
	struct mac_bsdextended_rule rule;
	int error, rulenum;

	error = bsde_parse_rule(argc, argv, &rule, BUFSIZ, errstr);
	if (error) {
		warnx("%s", errstr);
		return;
	}

	error = bsde_add_rule(&rulenum, &rule, BUFSIZ, errstr);
	if (error) {
		warnx("%s", errstr);
		return;
	}
	if (bsde_rule_to_string(&rule, charstr, BUFSIZ) == -1)
		warnx("Added rule, but unable to print string.");
	else
		printf("%d %s\n", rulenum, charstr);
}

void
list_rules(void)
{
	char errstr[BUFSIZ], charstr[BUFSIZ];
	struct mac_bsdextended_rule rule;
	int error, i, rule_count, rule_slots;

	rule_slots = bsde_get_rule_slots(BUFSIZ, errstr);
	if (rule_slots == -1) {
		warnx("unable to get rule slots; mac_bsdextended.ko "
		    "may not be loaded");
		errx(1, "bsde_get_rule_slots: %s", errstr);
	}

	rule_count = bsde_get_rule_count(BUFSIZ, errstr);
	if (rule_count == -1)
		errx(1, "bsde_get_rule_count: %s", errstr);

	printf("%d slots, %d rules\n", rule_slots, rule_count);

	for (i = 0; i < rule_slots; i++) {
		error = bsde_get_rule(i, &rule, BUFSIZ, errstr);
		switch (error) {
		case -2:
			continue;
		case -1:
			warnx("rule %d: %s", i, errstr);
			continue;
		case 0:
			break;
		}

		if (bsde_rule_to_string(&rule, charstr, BUFSIZ) == -1)
			warnx("unable to translate rule %d to string", i);
		else
			printf("%d %s\n", i, charstr);
	}
}

void
set_rule(int argc, char *argv[])
{
	char errstr[BUFSIZ];
	struct mac_bsdextended_rule rule;
	long value;
	int error, rulenum;
	char *endp;

	if (argc < 1)
		usage();

	value = strtol(argv[0], &endp, 10);
	if (*endp != '\0')
		usage();

	if ((long) value != (int) value || value < 0)
		usage();

	rulenum = value;

	error = bsde_parse_rule(argc - 1, argv + 1, &rule, BUFSIZ, errstr);
	if (error) {
		warnx("%s", errstr);
		return;
	}

	error = bsde_set_rule(rulenum, &rule, BUFSIZ, errstr);
	if (error) {
		warnx("%s", errstr);
		return;
	}
}

void
remove_rule(int argc, char *argv[])
{
	char errstr[BUFSIZ];
	long value;
	int error, rulenum;
	char *endp;

	if (argc != 1)
		usage();

	value = strtol(argv[0], &endp, 10);
	if (*endp != '\0')  
		usage();

	if ((long) value != (int) value || value < 0)
		usage();

	rulenum = value;

	error = bsde_delete_rule(rulenum, BUFSIZ, errstr);
	if (error)
		warnx("%s", errstr);
}

int
main(int argc, char *argv[])
{

	if (argc < 2)
		usage();

	if (strcmp("add", argv[1]) == 0) {
		add_rule(argc-2, argv+2);
	} else if (strcmp("list", argv[1]) == 0) {
		if (argc != 2)
			usage();
		list_rules();
	} else if (strcmp("set", argv[1]) == 0) {
		set_rule(argc-2, argv+2);
	} else if (strcmp("remove", argv[1]) == 0) {
		remove_rule(argc-2, argv+2);
	} else
		usage();

	return (0);
}