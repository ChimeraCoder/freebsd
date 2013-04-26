
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

#include "ftpd_locl.h"

RCSID("$Id$");

#if defined(KRB5)

int do_destroy_tickets = 1;
char *k5ccname;

#endif

#ifdef KRB5

static void
dest_cc(void)
{
    krb5_context context;
    krb5_error_code ret;
    krb5_ccache id;

    ret = krb5_init_context(&context);
    if (ret == 0) {
	if (k5ccname)
	    ret = krb5_cc_resolve(context, k5ccname, &id);
	else
	    ret = krb5_cc_default (context, &id);
	if (ret)
	    krb5_free_context(context);
    }
    if (ret == 0) {
	krb5_cc_destroy(context, id);
	krb5_free_context (context);
    }
}
#endif

#if defined(KRB5)

/*
 * Only destroy if we created the tickets
 */

void
cond_kdestroy(void)
{
    if (do_destroy_tickets) {
#if KRB5
	dest_cc();
#endif
	do_destroy_tickets = 0;
    }
    afsunlog();
}

void
kdestroy(void)
{
#if KRB5
    dest_cc();
#endif
    afsunlog();
    reply(200, "Tickets destroyed");
}


void
afslog(const char *cell, int quiet)
{
    if(k_hasafs()) {
#ifdef KRB5
	krb5_context context;
	krb5_error_code ret;
	krb5_ccache id;

	ret = krb5_init_context(&context);
	if (ret == 0) {
	    if (k5ccname)
		ret = krb5_cc_resolve(context, k5ccname, &id);
	    else
		ret = krb5_cc_default(context, &id);
	    if (ret)
		krb5_free_context(context);
	}
	if (ret == 0) {
	    krb5_afslog(context, id, cell, 0);
	    krb5_cc_close (context, id);
	    krb5_free_context (context);
	}
#endif
	if (!quiet)
	    reply(200, "afslog done");
    } else {
	if (!quiet)
	    reply(200, "no AFS present");
    }
}

void
afsunlog(void)
{
    if(k_hasafs())
	k_unlog();
}

#else
int ftpd_afslog_placeholder;
#endif /* KRB5 */