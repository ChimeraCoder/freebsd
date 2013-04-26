
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

#include <sys/types.h>
#include <sys/rctl.h>
#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <libutil.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	RCTL_DEFAULT_BUFSIZE	4096

static id_t
parse_user(const char *s)
{
	id_t id;
	char *end;
	struct passwd *pwd;

	pwd = getpwnam(s);
	if (pwd != NULL)
		return (pwd->pw_uid);

	if (!isnumber(s[0]))
		errx(1, "uknown user '%s'", s);

	id = strtod(s, &end);
	if ((size_t)(end - s) != strlen(s))
		errx(1, "trailing characters after numerical id");

	return (id);
}

static id_t
parse_group(const char *s)
{
	id_t id;
	char *end;
	struct group *grp;

	grp = getgrnam(s);
	if (grp != NULL)
		return (grp->gr_gid);

	if (!isnumber(s[0]))
		errx(1, "uknown group '%s'", s);

	id = strtod(s, &end);
	if ((size_t)(end - s) != strlen(s))
		errx(1, "trailing characters after numerical id");

	return (id);
}

/*
 * This routine replaces user/group name with numeric id.
 */
static char *
resolve_ids(char *rule)
{
	id_t id;
	const char *subject, *textid, *rest;
	char *resolved;

	subject = strsep(&rule, ":");
	textid = strsep(&rule, ":");
	if (textid == NULL)
		errx(1, "error in rule specification -- no subject");
	if (rule != NULL)
		rest = rule;
	else
		rest = "";

	if (strcasecmp(subject, "u") == 0)
		subject = "user";
	else if (strcasecmp(subject, "g") == 0)
		subject = "group";
	else if (strcasecmp(subject, "p") == 0)
		subject = "process";
	else if (strcasecmp(subject, "l") == 0 ||
	    strcasecmp(subject, "c") == 0 ||
	    strcasecmp(subject, "class") == 0)
		subject = "loginclass";
	else if (strcasecmp(subject, "j") == 0)
		subject = "jail";

	if (strcasecmp(subject, "user") == 0 && strlen(textid) > 0) {
		id = parse_user(textid);
		asprintf(&resolved, "%s:%d:%s", subject, (int)id, rest);
	} else if (strcasecmp(subject, "group") == 0 && strlen(textid) > 0) {
		id = parse_group(textid);
		asprintf(&resolved, "%s:%d:%s", subject, (int)id, rest);
	} else
		asprintf(&resolved, "%s:%s:%s", subject, textid, rest);

	if (resolved == NULL)
		err(1, "asprintf");

	return (resolved);
}

/*
 * This routine replaces "human-readable" number with its expanded form.
 */
static char *
expand_amount(char *rule)
{
	uint64_t num;
	const char *subject, *subject_id, *resource, *action, *amount, *per;
	char *copy, *expanded;

	copy = strdup(rule);
	if (copy == NULL)
		err(1, "strdup");

	subject = strsep(&copy, ":");
	subject_id = strsep(&copy, ":");
	resource = strsep(&copy, ":");
	action = strsep(&copy, "=/");
	amount = strsep(&copy, "/");
	per = copy;

	if (amount == NULL || strlen(amount) == 0) {
		free(copy);
		return (rule);
	}

	assert(subject != NULL);
	assert(subject_id != NULL);
	assert(resource != NULL);
	assert(action != NULL);

	if (expand_number(amount, &num))
		err(1, "expand_number");

	if (per == NULL)
		asprintf(&expanded, "%s:%s:%s:%s=%ju", subject, subject_id,
		    resource, action, (uintmax_t)num);
	else
		asprintf(&expanded, "%s:%s:%s:%s=%ju/%s", subject, subject_id,
		    resource, action, (uintmax_t)num, per);

	if (expanded == NULL)
		err(1, "asprintf");

	return (expanded);
}

static char *
humanize_ids(char *rule)
{
	id_t id;
	struct passwd *pwd;
	struct group *grp;
	const char *subject, *textid, *rest;
	char *humanized;

	subject = strsep(&rule, ":");
	textid = strsep(&rule, ":");
	if (textid == NULL)
		errx(1, "rule passed from the kernel didn't contain subject");
	if (rule != NULL)
		rest = rule;
	else
		rest = "";

	/* Replace numerical user and group ids with names. */
	if (strcasecmp(subject, "user") == 0) {
		id = parse_user(textid);
		pwd = getpwuid(id);
		if (pwd != NULL)
			textid = pwd->pw_name;
	} else if (strcasecmp(subject, "group") == 0) {
		id = parse_group(textid);
		grp = getgrgid(id);
		if (grp != NULL)
			textid = grp->gr_name;
	}

	asprintf(&humanized, "%s:%s:%s", subject, textid, rest);

	if (humanized == NULL)
		err(1, "asprintf");

	return (humanized);
}

static int
str2int64(const char *str, int64_t *value)
{
	char *end;

	if (str == NULL)
		return (EINVAL);

	*value = strtoul(str, &end, 10);
	if ((size_t)(end - str) != strlen(str))
		return (EINVAL);

	return (0);
}

static char *
humanize_amount(char *rule)
{
	int64_t num;
	const char *subject, *subject_id, *resource, *action, *amount, *per;
	char *copy, *humanized, buf[6];

	copy = strdup(rule);
	if (copy == NULL)
		err(1, "strdup");

	subject = strsep(&copy, ":");
	subject_id = strsep(&copy, ":");
	resource = strsep(&copy, ":");
	action = strsep(&copy, "=/");
	amount = strsep(&copy, "/");
	per = copy;

	if (amount == NULL || strlen(amount) == 0 ||
	    str2int64(amount, &num) != 0) {
		free(copy);
		return (rule);
	}

	assert(subject != NULL);
	assert(subject_id != NULL);
	assert(resource != NULL);
	assert(action != NULL);

	if (humanize_number(buf, sizeof(buf), num, "", HN_AUTOSCALE,
	    HN_DECIMAL | HN_NOSPACE) == -1)
		err(1, "humanize_number");

	if (per == NULL)
		asprintf(&humanized, "%s:%s:%s:%s=%s", subject, subject_id,
		    resource, action, buf);
	else
		asprintf(&humanized, "%s:%s:%s:%s=%s/%s", subject, subject_id,
		    resource, action, buf, per);

	if (humanized == NULL)
		err(1, "asprintf");

	return (humanized);
}

/*
 * Print rules, one per line.
 */
static void
print_rules(char *rules, int hflag, int nflag)
{
	char *rule;

	while ((rule = strsep(&rules, ",")) != NULL) {
		if (rule[0] == '\0')
			break; /* XXX */
		if (nflag == 0)
			rule = humanize_ids(rule);
		if (hflag)
			rule = humanize_amount(rule);
		printf("%s\n", rule);
	}
}

static void
add_rule(char *rule)
{
	int error;

	error = rctl_add_rule(rule, strlen(rule) + 1, NULL, 0);
	if (error != 0)
		err(1, "rctl_add_rule");
	free(rule);
}

static void
show_limits(char *filter, int hflag, int nflag)
{
	int error;
	char *outbuf = NULL;
	size_t outbuflen = RCTL_DEFAULT_BUFSIZE / 4;

	do {
		outbuflen *= 4;
		outbuf = realloc(outbuf, outbuflen);
		if (outbuf == NULL)
			err(1, "realloc");

		error = rctl_get_limits(filter, strlen(filter) + 1, outbuf,
		    outbuflen);
		if (error && errno != ERANGE)
			err(1, "rctl_get_limits");
	} while (error && errno == ERANGE);

	print_rules(outbuf, hflag, nflag);
	free(filter);
	free(outbuf);
}

static void
remove_rule(char *filter)
{
	int error;

	error = rctl_remove_rule(filter, strlen(filter) + 1, NULL, 0);
	if (error != 0)
		err(1, "rctl_remove_rule");
	free(filter);
}

static char *
humanize_usage_amount(char *usage)
{
	int64_t num;
	const char *resource, *amount;
	char *copy, *humanized, buf[6];

	copy = strdup(usage);
	if (copy == NULL)
		err(1, "strdup");

	resource = strsep(&copy, "=");
	amount = copy;

	assert(resource != NULL);
	assert(amount != NULL);

	if (str2int64(amount, &num) != 0 || 
	    humanize_number(buf, sizeof(buf), num, "", HN_AUTOSCALE,
	    HN_DECIMAL | HN_NOSPACE) == -1) {
		free(copy);
		return (usage);
	}

	asprintf(&humanized, "%s=%s", resource, buf);
	if (humanized == NULL)
		err(1, "asprintf");

	return (humanized);
}

/*
 * Query the kernel about a resource usage and print it out.
 */
static void
show_usage(char *filter, int hflag)
{
	int error;
	char *outbuf = NULL, *tmp;
	size_t outbuflen = RCTL_DEFAULT_BUFSIZE / 4;

	do {
		outbuflen *= 4;
		outbuf = realloc(outbuf, outbuflen);
		if (outbuf == NULL)
			err(1, "realloc");

		error = rctl_get_racct(filter, strlen(filter) + 1, outbuf,
		    outbuflen);
		if (error && errno != ERANGE)
			err(1, "rctl_get_racct");
	} while (error && errno == ERANGE);

	while ((tmp = strsep(&outbuf, ",")) != NULL) {
		if (tmp[0] == '\0')
			break; /* XXX */

		if (hflag)
			tmp = humanize_usage_amount(tmp);

		printf("%s\n", tmp);
	}

	free(filter);
	free(outbuf);
}

/*
 * Query the kernel about resource limit rules and print them out.
 */
static void
show_rules(char *filter, int hflag, int nflag)
{
	int error;
	char *outbuf = NULL;
	size_t filterlen, outbuflen = RCTL_DEFAULT_BUFSIZE / 4;

	if (filter != NULL)
		filterlen = strlen(filter) + 1;
	else
		filterlen = 0;

	do {
		outbuflen *= 4;
		outbuf = realloc(outbuf, outbuflen);
		if (outbuf == NULL)
			err(1, "realloc");

		error = rctl_get_rules(filter, filterlen, outbuf, outbuflen);
		if (error && errno != ERANGE)
			err(1, "rctl_get_rules");
	} while (error && errno == ERANGE);

	print_rules(outbuf, hflag, nflag);
	free(outbuf);
}

static void
usage(void)
{

	fprintf(stderr, "usage: rctl [ -h ] [-a rule | -l filter | -r filter "
	    "| -u filter | filter]\n");
	exit(1);
}

int
main(int argc __unused, char **argv __unused)
{
	int ch, aflag = 0, hflag = 0, nflag = 0, lflag = 0, rflag = 0,
	    uflag = 0;
	char *rule = NULL;

	while ((ch = getopt(argc, argv, "a:hl:nr:u:")) != -1) {
		switch (ch) {
		case 'a':
			aflag = 1;
			rule = strdup(optarg);
			break;
		case 'h':
			hflag = 1;
			break;
		case 'l':
			lflag = 1;
			rule = strdup(optarg);
			break;
		case 'n':
			nflag = 1;
			break;
		case 'r':
			rflag = 1;
			rule = strdup(optarg);
			break;
		case 'u':
			uflag = 1;
			rule = strdup(optarg);
			break;

		case '?':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 1)
		usage();

	if (rule == NULL) {
		if (argc == 1)
			rule = strdup(argv[0]);
		else
			rule = strdup("::");
	}

	if (aflag + lflag + rflag + uflag + argc > 1)
		errx(1, "only one flag or argument may be specified "
		    "at the same time");

	rule = resolve_ids(rule);
	rule = expand_amount(rule);

	if (aflag) {
		add_rule(rule);
		return (0);
	}

	if (lflag) {
		show_limits(rule, hflag, nflag);
		return (0);
	}

	if (rflag) {
		remove_rule(rule);
		return (0);
	}

	if (uflag) {
		show_usage(rule, hflag);
		return (0);
	}

	show_rules(rule, hflag, nflag);
	return (0);
}