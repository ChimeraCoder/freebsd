
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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <unistd.h>

#include "atf-c/build.h"
#include "atf-c/check.h"
#include "atf-c/config.h"
#include "atf-c/error.h"
#include "atf-c/macros.h"

#include "dynstr.h"
#include "fs.h"
#include "process.h"
#include "test_helpers.h"

static
void
build_check_c_o_aux(const char *path, const char *failmsg,
                    const bool expect_pass)
{
    bool success;
    atf_dynstr_t iflag;
    const char *optargs[4];

    RE(atf_dynstr_init_fmt(&iflag, "-I%s", atf_config_get("atf_includedir")));

    optargs[0] = atf_dynstr_cstring(&iflag);
    optargs[1] = "-Wall";
    optargs[2] = "-Werror";
    optargs[3] = NULL;

    RE(atf_check_build_c_o(path, "test.o", optargs, &success));

    atf_dynstr_fini(&iflag);

    if ((expect_pass && !success) || (!expect_pass && success))
        atf_tc_fail("%s", failmsg);
}

void
build_check_c_o(const atf_tc_t *tc, const char *sfile, const char *failmsg,
                const bool expect_pass)
{
    atf_fs_path_t path;

    RE(atf_fs_path_init_fmt(&path, "%s/%s",
                            atf_tc_get_config_var(tc, "srcdir"), sfile));
    build_check_c_o_aux(atf_fs_path_cstring(&path), failmsg, expect_pass);
    atf_fs_path_fini(&path);
}

void
header_check(const char *hdrname)
{
    FILE *srcfile;
    char failmsg[128];

    srcfile = fopen("test.c", "w");
    ATF_REQUIRE(srcfile != NULL);
    fprintf(srcfile, "#include <%s>\n", hdrname);
    fclose(srcfile);

    snprintf(failmsg, sizeof(failmsg),
             "Header check failed; %s is not self-contained", hdrname);

    build_check_c_o_aux("test.c", failmsg, true);
}

void
get_process_helpers_path(const atf_tc_t *tc, const bool is_detail,
                         atf_fs_path_t *path)
{
    RE(atf_fs_path_init_fmt(path, "%s/%sprocess_helpers",
                            atf_tc_get_config_var(tc, "srcdir"),
                            is_detail ? "" : "detail/"));
}

bool
grep_string(const atf_dynstr_t *str, const char *regex)
{
    int res;
    regex_t preg;

    printf("Looking for '%s' in '%s'\n", regex, atf_dynstr_cstring(str));
    ATF_REQUIRE(regcomp(&preg, regex, REG_EXTENDED) == 0);

    res = regexec(&preg, atf_dynstr_cstring(str), 0, NULL, 0);
    ATF_REQUIRE(res == 0 || res == REG_NOMATCH);

    regfree(&preg);

    return res == 0;
}

bool
grep_file(const char *file, const char *regex, ...)
{
    bool done, found;
    int fd;
    va_list ap;
    atf_dynstr_t formatted;

    va_start(ap, regex);
    RE(atf_dynstr_init_ap(&formatted, regex, ap));
    va_end(ap);

    done = false;
    found = false;
    ATF_REQUIRE((fd = open(file, O_RDONLY)) != -1);
    do {
        atf_dynstr_t line;

        RE(atf_dynstr_init(&line));

        done = read_line(fd, &line);
        if (!done)
            found = grep_string(&line, atf_dynstr_cstring(&formatted));

        atf_dynstr_fini(&line);
    } while (!found && !done);
    close(fd);

    atf_dynstr_fini(&formatted);

    return found;
}

bool
read_line(int fd, atf_dynstr_t *dest)
{
    char ch;
    ssize_t cnt;

    while ((cnt = read(fd, &ch, sizeof(ch))) == sizeof(ch) &&
           ch != '\n') {
        const atf_error_t err = atf_dynstr_append_fmt(dest, "%c", ch);
        ATF_REQUIRE(!atf_is_error(err));
    }
    ATF_REQUIRE(cnt != -1);

    return cnt == 0;
}

struct run_h_tc_data {
    atf_tc_t *m_tc;
    const char *m_resname;
};

static
void
run_h_tc_child(void *v)
{
    struct run_h_tc_data *data = (struct run_h_tc_data *)v;

    RE(atf_tc_run(data->m_tc, data->m_resname));
}

/* TODO: Investigate if it's worth to add this functionality as part of
 * the public API.  I.e. a function to easily run a test case body in a
 * subprocess. */
void
run_h_tc(atf_tc_t *tc, const char *outname, const char *errname,
         const char *resname)
{
    atf_fs_path_t outpath, errpath;
    atf_process_stream_t outb, errb;
    atf_process_child_t child;
    atf_process_status_t status;

    RE(atf_fs_path_init_fmt(&outpath, outname));
    RE(atf_fs_path_init_fmt(&errpath, errname));

    struct run_h_tc_data data = { tc, resname };

    RE(atf_process_stream_init_redirect_path(&outb, &outpath));
    RE(atf_process_stream_init_redirect_path(&errb, &errpath));
    RE(atf_process_fork(&child, run_h_tc_child, &outb, &errb, &data));
    atf_process_stream_fini(&errb);
    atf_process_stream_fini(&outb);

    RE(atf_process_child_wait(&child, &status));
    ATF_CHECK(atf_process_status_exited(&status));
    atf_process_status_fini(&status);

    atf_fs_path_fini(&errpath);
    atf_fs_path_fini(&outpath);
}