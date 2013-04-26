
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
#include <stdlib.h>
#include <string.h>
#include <com_right.h>

#ifdef LIBINTL
#include <libintl.h>
#else
#define dgettext(d,s) (s)
#endif

KRB5_LIB_FUNCTION const char * KRB5_LIB_CALL
com_right(struct et_list *list, long code)
{
    struct et_list *p;
    for (p = list; p; p = p->next)
	if (code >= p->table->base && code < p->table->base + p->table->n_msgs)
	    return p->table->msgs[code - p->table->base];
    return NULL;
}

KRB5_LIB_FUNCTION const char * KRB5_LIB_CALL
com_right_r(struct et_list *list, long code, char *str, size_t len)
{
    struct et_list *p;
    for (p = list; p; p = p->next) {
	if (code >= p->table->base && code < p->table->base + p->table->n_msgs) {
	    const char *msg = p->table->msgs[code - p->table->base];
#ifdef LIBINTL
	    char domain[12 + 20];
	    snprintf(domain, sizeof(domain), "heim_com_err%d", p->table->base);
#endif
	    strlcpy(str, dgettext(domain, msg), len);
	    return str;
	}
    }
    return NULL;
}

struct foobar {
    struct et_list etl;
    struct error_table et;
};

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
initialize_error_table_r(struct et_list **list,
			 const char **messages,
			 int num_errors,
			 long base)
{
    struct et_list *et, **end;
    struct foobar *f;
    for (end = list, et = *list; et; end = &et->next, et = et->next)
        if (et->table->msgs == messages)
            return;
    f = malloc(sizeof(*f));
    if (f == NULL)
        return;
    et = &f->etl;
    et->table = &f->et;
    et->table->msgs = messages;
    et->table->n_msgs = num_errors;
    et->table->base = base;
    et->next = NULL;
    *end = et;
}


KRB5_LIB_FUNCTION void KRB5_LIB_CALL
free_error_table(struct et_list *et)
{
    while(et){
	struct et_list *p = et;
	et = et->next;
	free(p);
    }
}