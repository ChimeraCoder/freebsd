
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

#include "gen_locl.h"

RCSID("$Id$");

static void
generate_2int (const Type *t, const char *gen_name)
{
    Member *m;

    fprintf (headerfile,
	     "unsigned %s2int(%s);\n",
	     gen_name, gen_name);

    fprintf (codefile,
	     "unsigned %s2int(%s f)\n"
	     "{\n"
	     "unsigned r = 0;\n",
	     gen_name, gen_name);

    ASN1_TAILQ_FOREACH(m, t->members, members) {
	fprintf (codefile, "if(f.%s) r |= (1U << %d);\n",
		 m->gen_name, m->val);
    }
    fprintf (codefile, "return r;\n"
	     "}\n\n");
}

static void
generate_int2 (const Type *t, const char *gen_name)
{
    Member *m;

    fprintf (headerfile,
	     "%s int2%s(unsigned);\n",
	     gen_name, gen_name);

    fprintf (codefile,
	     "%s int2%s(unsigned n)\n"
	     "{\n"
	     "\t%s flags;\n\n"
	     "\tmemset(&flags, 0, sizeof(flags));\n\n",
	     gen_name, gen_name, gen_name);

    if(t->members) {
	ASN1_TAILQ_FOREACH(m, t->members, members) {
	    fprintf (codefile, "\tflags.%s = (n >> %d) & 1;\n",
		     m->gen_name, m->val);
	}
    }
    fprintf (codefile, "\treturn flags;\n"
	     "}\n\n");
}

/*
 * This depends on the bit string being declared in increasing order
 */

static void
generate_units (const Type *t, const char *gen_name)
{
    Member *m;

    if (template_flag) {
	fprintf (headerfile,
		 "extern const struct units *asn1_%s_table_units;\n",
		 gen_name);
	fprintf (headerfile, "#define asn1_%s_units() (asn1_%s_table_units)\n",
		 gen_name, gen_name);
    } else {
	fprintf (headerfile,
		 "const struct units * asn1_%s_units(void);\n",
		 gen_name);
    }

    fprintf (codefile,
	     "static struct units %s_units[] = {\n",
	     gen_name);

    if(t->members) {
	ASN1_TAILQ_FOREACH_REVERSE(m, t->members, memhead, members) {
	    fprintf (codefile,
		     "\t{\"%s\",\t1U << %d},\n", m->name, m->val);
	}
    }

    fprintf (codefile,
	     "\t{NULL,\t0}\n"
	     "};\n\n");

    if (template_flag)
	fprintf (codefile,
		 "const struct units * asn1_%s_table_units = %s_units;\n",
		 gen_name, gen_name);
    else
	fprintf (codefile,
		 "const struct units * asn1_%s_units(void){\n"
		 "return %s_units;\n"
		 "}\n\n",
		 gen_name, gen_name);


}

void
generate_glue (const Type *t, const char *gen_name)
{
    switch(t->type) {
    case TTag:
	generate_glue(t->subtype, gen_name);
	break;
    case TBitString :
	if (!ASN1_TAILQ_EMPTY(t->members)) {
	    generate_2int (t, gen_name);
	    generate_int2 (t, gen_name);
	    generate_units (t, gen_name);
	}
	break;
    default :
	break;
    }
}