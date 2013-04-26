
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

#include <stdio.h>
#include <openssl/conf.h>

static void print_conf(CONF_VALUE *cv);
static IMPLEMENT_LHASH_DOALL_FN(print_conf, CONF_VALUE *);

main()
	{
	LHASH *conf;
	long l;

	conf=CONF_load(NULL,"../../apps/openssl.cnf",&l);
	if (conf == NULL)
		{
		fprintf(stderr,"error loading config, line %ld\n",l);
		exit(1);
		}

	lh_doall(conf,LHASH_DOALL_FN(print_conf));
	}


static void print_conf(CONF_VALUE *cv)
	{
	int i;
	CONF_VALUE *v;
	char *section;
	char *name;
	char *value;
	STACK *s;

	/* If it is a single entry, return */

	if (cv->name != NULL) return;

	printf("[ %s ]\n",cv->section);
	s=(STACK *)cv->value;

	for (i=0; i<sk_num(s); i++)
		{
		v=(CONF_VALUE *)sk_value(s,i);
		section=(v->section == NULL)?"None":v->section;
		name=(v->name == NULL)?"None":v->name;
		value=(v->value == NULL)?"None":v->value;
		printf("%s=%s\n",name,value);
		}
	printf("\n");
	}