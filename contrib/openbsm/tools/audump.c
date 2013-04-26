
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

#include <bsm/libbsm.h>
#include <string.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Simple tool to dump various /etc/security databases using the defined APIs.
 */

static void
usage(void)
{

	fprintf(stderr, "usage: audump [class|class_r|control|event|event_r|"
	    "user|user_r]\n");
	exit(-1);
}

static void
audump_class(void)
{
	au_class_ent_t *cp;

	while ((cp = getauclassent()) != NULL)
		printf("0x%08x:%s:%s\n", cp->ac_class, cp->ac_name,
		    cp->ac_desc);
}

static void
audump_class_r(void)
{
	char class_ent_name[AU_CLASS_NAME_MAX];
	char class_ent_desc[AU_CLASS_DESC_MAX];
	au_class_ent_t c, *cp;

	bzero(&c, sizeof(c));
	bzero(class_ent_name, sizeof(class_ent_name));
	bzero(class_ent_desc, sizeof(class_ent_desc));
	c.ac_name = class_ent_name;
	c.ac_desc = class_ent_desc;

	while ((cp = getauclassent_r(&c)) != NULL)
		printf("0x%08x:%s:%s\n", cp->ac_class, cp->ac_name,
		    cp->ac_desc);
}

static void
audump_control(void)
{
	char string[PATH_MAX], string2[PATH_MAX];
	int ret, val;
	long policy;
	time_t age;
	size_t size;

	ret = getacflg(string, PATH_MAX);
	if (ret == -2)
		err(-1, "getacflg");
	if (ret != 0)
		errx(-1, "getacflg: %d", ret);

	printf("flags:%s\n", string);

	ret = getacmin(&val);
	if (ret == -2)
		err(-1, "getacmin");
	if (ret != 0)
		errx(-1, "getacmin: %d", ret);

	printf("min:%d\n", val);

	ret = getacna(string, PATH_MAX);
	if (ret == -2)
		err(-1, "getacna");
	if (ret != 0)
		errx(-1, "getacna: %d", ret);

	printf("naflags:%s\n", string);

	setac();
	do {
		ret = getacdir(string, PATH_MAX);
		if (ret == -1)
			break;
		if (ret == -2)
			err(-1, "getacdir");
		if (ret != 0)
			errx(-1, "getacdir: %d", ret);
		printf("dir:%s\n", string);

	} while (ret == 0);

	ret = getacpol(string, PATH_MAX);
	if (ret != 0)
		err(-1, "getacpol");
	if (au_strtopol(string, &policy) < 0)
		err(-1, "au_strtopol");
	if (au_poltostr(policy, PATH_MAX, string2) < 0)
		err(-1, "au_poltostr");
	printf("policy:%s\n", string2);

	ret = getacfilesz(&size);
	if (ret == -2)
		err(-1, "getacfilesz");
	if (ret != 0)
		err(-1, "getacfilesz: %d", ret);

	printf("filesz:%ldB\n", size);


	ret = getachost(string, PATH_MAX);
	if (ret == -2)
		err(-1, "getachost");
	if (ret == -3)
		err(-1, "getachost: %d", ret);
	if (ret == 0 && ret != 1)
		printf("host:%s\n", string);

	ret = getacexpire(&val, &age, &size);
	if (ret == -2)
		err(-1, "getacexpire");
	if (ret == -1)
		err(-1, "getacexpire: %d", ret);
	if (ret == 0 && ret != 1)
		printf("expire-after:%ldB  %s %lds\n", size,
		    val ? "AND" : "OR", age);
}

static void
printf_classmask(au_class_t classmask)
{
	au_class_ent_t *c;
	u_int32_t i;
	int first;

	first = 1;
	for (i = 0; i < 32; i++) {
		if (classmask & (1 << i)) {
			if (first)
				first = 0;
			else
				printf(",");
			c = getauclassnum(1 << i);
			if (c != NULL)
				printf("%s", c->ac_name);
			else
				printf("0x%x", 1 << i);
		}
	}
}

static void
audump_event(void)
{
	au_event_ent_t *ep;

	while ((ep = getauevent()) != NULL) {
		printf("%d:%s:%s:", ep->ae_number, ep->ae_name, ep->ae_desc);
		printf_classmask(ep->ae_class);
		printf("\n");
	}
}

static void
audump_event_r(void)
{
	char event_ent_name[AU_EVENT_NAME_MAX];
	char event_ent_desc[AU_EVENT_DESC_MAX];
	au_event_ent_t e, *ep;

	bzero(&e, sizeof(e));
	bzero(event_ent_name, sizeof(event_ent_name));
	bzero(event_ent_desc, sizeof(event_ent_desc));
	e.ae_name = event_ent_name;
	e.ae_desc = event_ent_desc;

	while ((ep = getauevent_r(&e)) != NULL) {
		printf("%d:%s:%s:", ep->ae_number, ep->ae_name, ep->ae_desc);
		printf_classmask(ep->ae_class);
		printf("\n");
	}
}

static void
audump_user(void)
{
	au_user_ent_t *up;

	while ((up = getauuserent()) != NULL) {
		printf("%s:", up->au_name);
		// printf_classmask(up->au_always);
		printf(":");
		// printf_classmask(up->au_never);
		printf("\n");
	}
}

static void
audump_user_r(void)
{
	char user_ent_name[AU_USER_NAME_MAX];
	au_user_ent_t u, *up;

	bzero(&u, sizeof(u));
	bzero(user_ent_name, sizeof(user_ent_name));
	u.au_name = user_ent_name;

	while ((up = getauuserent_r(&u)) != NULL) {
		printf("%s:", up->au_name);
		// printf_classmask(up->au_always);
		printf(":");
		// printf_classmask(up->au_never);
		printf("\n");
	}
}

int
main(int argc, char *argv[])
{

	if (argc != 2)
		usage();

	if (strcmp(argv[1], "class") == 0)
		audump_class();
	else if (strcmp(argv[1], "class_r") == 0)
		audump_class_r();
	else if (strcmp(argv[1], "control") == 0)
		audump_control();
	else if (strcmp(argv[1], "event") == 0)
		audump_event();
	else if (strcmp(argv[1], "event_r") == 0)
		audump_event_r();
	else if (strcmp(argv[1], "user") == 0)
		audump_user();
	else if (strcmp(argv[1], "user_r") == 0)
		audump_user_r();
	else
		usage();

	return (0);
}