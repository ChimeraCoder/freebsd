
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

#include "ktutil_locl.h"

RCSID("$Id$");

/*
 * keep track of the highest version for every principal.
 */

struct e {
    krb5_principal principal;
    int max_vno;
    time_t timestamp;
    struct e *next;
};

static struct e *
get_entry (krb5_principal princ, struct e *head)
{
    struct e *e;

    for (e = head; e != NULL; e = e->next)
	if (krb5_principal_compare (context, princ, e->principal))
	    return e;
    return NULL;
}

static void
add_entry (krb5_principal princ, int vno, time_t timestamp, struct e **head)
{
    krb5_error_code ret;
    struct e *e;

    e = get_entry (princ, *head);
    if (e != NULL) {
	if(e->max_vno < vno) {
	    e->max_vno = vno;
	    e->timestamp = timestamp;
	}
	return;
    }
    e = malloc (sizeof (*e));
    if (e == NULL)
	krb5_errx (context, 1, "malloc: out of memory");
    ret = krb5_copy_principal (context, princ, &e->principal);
    if (ret)
	krb5_err (context, 1, ret, "krb5_copy_principal");
    e->max_vno = vno;
    e->timestamp = timestamp;
    e->next    = *head;
    *head      = e;
}

static void
delete_list (struct e *head)
{
    while (head != NULL) {
	struct e *next = head->next;
	krb5_free_principal (context, head->principal);
	free (head);
	head = next;
    }
}

/*
 * Remove all entries that have newer versions and that are older
 * than `age'
 */

int
kt_purge(struct purge_options *opt, int argc, char **argv)
{
    krb5_error_code ret = 0;
    krb5_kt_cursor cursor;
    krb5_keytab keytab;
    krb5_keytab_entry entry;
    int age;
    struct e *head = NULL;
    time_t judgement_day;

    age = parse_time(opt->age_string, "s");
    if(age < 0) {
	krb5_warnx(context, "unparasable time `%s'", opt->age_string);
	return 1;
    }

    if((keytab = ktutil_open_keytab()) == NULL)
	return 1;

    ret = krb5_kt_start_seq_get(context, keytab, &cursor);
    if(ret){
	krb5_warn(context, ret, "%s", keytab_string);
	goto out;
    }

    while(krb5_kt_next_entry(context, keytab, &entry, &cursor) == 0) {
	add_entry (entry.principal, entry.vno, entry.timestamp, &head);
	krb5_kt_free_entry(context, &entry);
    }
    krb5_kt_end_seq_get(context, keytab, &cursor);

    judgement_day = time (NULL);

    ret = krb5_kt_start_seq_get(context, keytab, &cursor);
    if(ret){
	krb5_warn(context, ret, "%s", keytab_string);
	goto out;
    }

    while(krb5_kt_next_entry(context, keytab, &entry, &cursor) == 0) {
	struct e *e = get_entry (entry.principal, head);

	if (e == NULL) {
	    krb5_warnx (context, "ignoring extra entry");
	    continue;
	}

	if (entry.vno < e->max_vno
	    && judgement_day - e->timestamp > age) {
	    if (verbose_flag) {
		char *name_str;

		krb5_unparse_name (context, entry.principal, &name_str);
		printf ("removing %s vno %d\n", name_str, entry.vno);
		free (name_str);
	    }
	    ret = krb5_kt_remove_entry (context, keytab, &entry);
	    if (ret)
		krb5_warn (context, ret, "remove");
	}
	krb5_kt_free_entry(context, &entry);
    }
    ret = krb5_kt_end_seq_get(context, keytab, &cursor);

    delete_list (head);

 out:
    krb5_kt_close (context, keytab);
    return ret != 0;
}