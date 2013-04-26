
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

#undef ROKEN_RENAME

#include "config.h"

#include "compile_et.h"
#include <getarg.h>

#include <roken.h>
#include <err.h>
#include "parse.h"

int numerror;
extern FILE *yyin;

extern void yyparse(void);

long base_id;
int number;
char *prefix;
char *id_str;

char name[128];
char Basename[128];

#ifdef YYDEBUG
extern int yydebug = 1;
#endif

char *filename;
char hfn[128];
char cfn[128];

struct error_code *codes = NULL;

static int
generate_c(void)
{
    int n;
    struct error_code *ec;

    FILE *c_file = fopen(cfn, "w");
    if(c_file == NULL)
	return 1;

    fprintf(c_file, "/* Generated from %s */\n", filename);
    if(id_str)
	fprintf(c_file, "/* %s */\n", id_str);
    fprintf(c_file, "\n");
    fprintf(c_file, "#include <stddef.h>\n");
    fprintf(c_file, "#include <com_err.h>\n");
    fprintf(c_file, "#include \"%s\"\n", hfn);
    fprintf(c_file, "\n");
    fprintf(c_file, "#define N_(x) (x)\n");
    fprintf(c_file, "\n");

    fprintf(c_file, "static const char *%s_error_strings[] = {\n", name);

    for(ec = codes, n = 0; ec; ec = ec->next, n++) {
	while(n < ec->number) {
	    fprintf(c_file, "\t/* %03d */ \"Reserved %s error (%d)\",\n",
		    n, name, n);
	    n++;

	}
	fprintf(c_file, "\t/* %03d */ N_(\"%s\"),\n",
		ec->number, ec->string);
    }

    fprintf(c_file, "\tNULL\n");
    fprintf(c_file, "};\n");
    fprintf(c_file, "\n");
    fprintf(c_file, "#define num_errors %d\n", number);
    fprintf(c_file, "\n");
    fprintf(c_file,
	    "void initialize_%s_error_table_r(struct et_list **list)\n",
	    name);
    fprintf(c_file, "{\n");
    fprintf(c_file,
	    "    initialize_error_table_r(list, %s_error_strings, "
	    "num_errors, ERROR_TABLE_BASE_%s);\n", name, name);
    fprintf(c_file, "}\n");
    fprintf(c_file, "\n");
    fprintf(c_file, "void initialize_%s_error_table(void)\n", name);
    fprintf(c_file, "{\n");
    fprintf(c_file,
	    "    init_error_table(%s_error_strings, ERROR_TABLE_BASE_%s, "
	    "num_errors);\n", name, name);
    fprintf(c_file, "}\n");

    fclose(c_file);
    return 0;
}

static int
generate_h(void)
{
    struct error_code *ec;
    char fn[128];
    FILE *h_file = fopen(hfn, "w");
    char *p;

    if(h_file == NULL)
	return 1;

    snprintf(fn, sizeof(fn), "__%s__", hfn);
    for(p = fn; *p; p++)
	if(!isalnum((unsigned char)*p))
	    *p = '_';

    fprintf(h_file, "/* Generated from %s */\n", filename);
    if(id_str)
	fprintf(h_file, "/* %s */\n", id_str);
    fprintf(h_file, "\n");
    fprintf(h_file, "#ifndef %s\n", fn);
    fprintf(h_file, "#define %s\n", fn);
    fprintf(h_file, "\n");
    fprintf(h_file, "struct et_list;\n");
    fprintf(h_file, "\n");
    fprintf(h_file,
	    "void initialize_%s_error_table_r(struct et_list **);\n",
	    name);
    fprintf(h_file, "\n");
    fprintf(h_file, "void initialize_%s_error_table(void);\n", name);
    fprintf(h_file, "#define init_%s_err_tbl initialize_%s_error_table\n",
	    name, name);
    fprintf(h_file, "\n");
    fprintf(h_file, "typedef enum %s_error_number{\n", name);

    for(ec = codes; ec; ec = ec->next) {
	fprintf(h_file, "\t%s = %ld%s\n", ec->name, base_id + ec->number,
		(ec->next != NULL) ? "," : "");
    }

    fprintf(h_file, "} %s_error_number;\n", name);
    fprintf(h_file, "\n");
    fprintf(h_file, "#define ERROR_TABLE_BASE_%s %ld\n", name, base_id);
    fprintf(h_file, "\n");
    fprintf(h_file, "#define COM_ERR_BINDDOMAIN_%s \"heim_com_err%ld\"\n", name, base_id);
    fprintf(h_file, "\n");
    fprintf(h_file, "#endif /* %s */\n", fn);


    fclose(h_file);
    return 0;
}

static int
generate(void)
{
    return generate_c() || generate_h();
}

int version_flag;
int help_flag;
struct getargs args[] = {
    { "version", 0, arg_flag, &version_flag },
    { "help", 0, arg_flag, &help_flag }
};
int num_args = sizeof(args) / sizeof(args[0]);

static void
usage(int code)
{
    arg_printusage(args, num_args, NULL, "error-table");
    exit(code);
}

int
main(int argc, char **argv)
{
    char *p;
    int optidx = 0;

    setprogname(argv[0]);
    if(getarg(args, num_args, argc, argv, &optidx))
	usage(1);
    if(help_flag)
	usage(0);
    if(version_flag) {
	print_version(NULL);
	exit(0);
    }

    if(optidx == argc)
	usage(1);
    filename = argv[optidx];
    yyin = fopen(filename, "r");
    if(yyin == NULL)
	err(1, "%s", filename);


    p = strrchr(filename, rk_PATH_DELIM);
    if(p)
	p++;
    else
	p = filename;
    strlcpy(Basename, p, sizeof(Basename));

    Basename[strcspn(Basename, ".")] = '\0';

    snprintf(hfn, sizeof(hfn), "%s.h", Basename);
    snprintf(cfn, sizeof(cfn), "%s.c", Basename);

    yyparse();
    if(numerror)
	return 1;

    return generate();
}