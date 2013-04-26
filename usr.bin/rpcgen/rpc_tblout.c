
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

#if 0
#ifndef lint
#ident	"@(#)rpc_tblout.c	1.11	93/07/05 SMI"
static char sccsid[] = "@(#)rpc_tblout.c 1.4 89/02/22 (C) 1988 SMI";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * rpc_tblout.c, Dispatch table outputter for the RPC protocol compiler
 * Copyright (C) 1989, Sun Microsystems, Inc.
 */
#include <err.h>
#include <stdio.h>
#include <string.h>
#include "rpc_parse.h"
#include "rpc_scan.h"
#include "rpc_util.h"

#define TABSIZE		8
#define TABCOUNT	5
#define TABSTOP		(TABSIZE*TABCOUNT)

static char tabstr[TABCOUNT+1] = "\t\t\t\t\t";

static char tbl_hdr[] = "struct rpcgen_table %s_table[] = {\n";
static char tbl_end[] = "};\n";

static char null_entry[] = "\n\t(char *(*)())0,\n\
 \t(xdrproc_t) xdr_void,\t\t\t0,\n\
 \t(xdrproc_t) xdr_void,\t\t\t0,\n";


static char tbl_nproc[] = "int %s_nproc =\n\tsizeof(%s_table)/sizeof(%s_table[0]);\n\n";

static void write_table( definition * );
static void printit(const  char *, const char *);

void
write_tables(void)
{
	list *l;
	definition *def;

	f_print(fout, "\n");
	for (l = defined; l != NULL; l = l->next) {
		def = (definition *) l->val;
		if (def->def_kind == DEF_PROGRAM) {
			write_table(def);
		}
	}
}

static void
write_table(definition *def)
{
	version_list *vp;
	proc_list *proc;
	int current;
	int expected;
	char progvers[100];
	int warning;

	for (vp = def->def.pr.versions; vp != NULL; vp = vp->next) {
		warning = 0;
		s_print(progvers, "%s_%s",
		    locase(def->def_name), vp->vers_num);
		/* print the table header */
		f_print(fout, tbl_hdr, progvers);

		if (nullproc(vp->procs)) {
			expected = 0;
		} else {
			expected = 1;
			fputs(null_entry, fout);
		}
		for (proc = vp->procs; proc != NULL; proc = proc->next) {
			current = atoi(proc->proc_num);
			if (current != expected++) {
				f_print(fout,
			"\n/*\n * WARNING: table out of order\n */\n");
				if (warning == 0) {
					warnx("WARNING %s table is out of order", progvers);
					warning = 1;
					nonfatalerrors = 1;
				}
				expected = current + 1;
			}
			f_print(fout, "\n\t(char *(*)())RPCGEN_ACTION(");

			/* routine to invoke */
			if( !newstyle )
			  pvname_svc(proc->proc_name, vp->vers_num);
			else {
			  if( newstyle )
			    f_print( fout, "_");   /* calls internal func */
			  pvname(proc->proc_name, vp->vers_num);
			}
			f_print(fout, "),\n");

			/* argument info */
			if( proc->arg_num > 1 )
			  printit((char*) NULL, proc->args.argname );
			else
			  /* do we have to do something special for newstyle */
			  printit( proc->args.decls->decl.prefix,
				  proc->args.decls->decl.type );
			/* result info */
			printit(proc->res_prefix, proc->res_type);
		}

		/* print the table trailer */
		fputs(tbl_end, fout);
		f_print(fout, tbl_nproc, progvers, progvers, progvers);
	}
}

static void
printit(const char *prefix, const char *type)
{
	int len;
	int tabs;


 	len = fprintf(fout, "\txdr_%s,", stringfix(type));
	/* account for leading tab expansion */
	len += TABSIZE - 1;
	/* round up to tabs required */
	tabs = (TABSTOP - len + TABSIZE - 1)/TABSIZE;
	f_print(fout, "%s", &tabstr[TABCOUNT-tabs]);

	if (streq(type, "void")) {
		f_print(fout, "0");
	} else {
		f_print(fout, "sizeof ( ");
		/* XXX: should "follow" be 1 ??? */
		ptype(prefix, type, 0);
		f_print(fout, ")");
	}
	f_print(fout, ",\n");
}