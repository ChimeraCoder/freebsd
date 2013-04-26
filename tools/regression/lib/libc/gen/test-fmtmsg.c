
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

#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fmtmsg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *run_test(long classification, const char *label, int severity,
    const char *text, const char *action, const char *tag);

struct testcase {
	long classification;
	const char *label;
	int severity;
	const char *text;
	const char *action;
	const char *tag;
	const char *msgverb;
	const char *result;
} testcases[] = {
	{
		MM_UTIL | MM_PRINT, "BSD:ls", MM_ERROR,
		"illegal option -- z", "refer to manual", "BSD:ls:001",
		NULL,
		"BSD:ls: ERROR: illegal option -- z\n"
		    "TO FIX: refer to manual BSD:ls:001\n"
	},
	{
		MM_UTIL | MM_PRINT, "BSD:ls", MM_ERROR,
		"illegal option -- z", "refer to manual", "BSD:ls:001",
		"text:severity:action:tag",
		"illegal option -- z: ERROR\n"
		    "TO FIX: refer to manual BSD:ls:001\n"
	},
	{
		MM_UTIL | MM_PRINT, "BSD:ls", MM_ERROR,
		"illegal option -- z", "refer to manual", "BSD:ls:001",
		"text",
		"illegal option -- z\n"
	},
	{
		MM_UTIL | MM_PRINT, "BSD:ls", MM_ERROR,
		"illegal option -- z", "refer to manual", "BSD:ls:001",
		"severity:text",
		"ERROR: illegal option -- z\n"
	},
	{
		MM_UTIL | MM_PRINT, "BSD:ls", MM_ERROR,
		"illegal option -- z", "refer to manual", "BSD:ls:001",
		"ignore me",
		"BSD:ls: ERROR: illegal option -- z\n"
		    "TO FIX: refer to manual BSD:ls:001\n"
	},
	{
		MM_UTIL | MM_PRINT, "BSD:ls", MM_ERROR,
		"illegal option -- z", "refer to manual", "BSD:ls:001",
		"tag:severity:text:nothing:action",
		"BSD:ls: ERROR: illegal option -- z\n"
		    "TO FIX: refer to manual BSD:ls:001\n"
	},
	{
		MM_UTIL | MM_PRINT, "BSD:ls", MM_ERROR,
		"illegal option -- z", "refer to manual", "BSD:ls:001",
		"",
		"BSD:ls: ERROR: illegal option -- z\n"
		    "TO FIX: refer to manual BSD:ls:001\n"
	},
	{
		MM_UTIL | MM_PRINT, MM_NULLLBL, MM_ERROR,
		"illegal option -- z", "refer to manual", "BSD:ls:001",
		NULL,
		"ERROR: illegal option -- z\n"
		    "TO FIX: refer to manual BSD:ls:001\n"
	},
	{
		MM_UTIL | MM_PRINT, "BSD:ls", MM_ERROR,
		"illegal option -- z", MM_NULLACT, MM_NULLTAG,
		NULL,
		"BSD:ls: ERROR: illegal option -- z\n"
	},
	{
		MM_UTIL | MM_NULLMC, "BSD:ls", MM_ERROR,
		"illegal option -- z", "refer to manual", "BSD:ls:001",
		NULL,
		""
	},
	{
		MM_APPL | MM_PRINT, "ABCDEFGHIJ:abcdefghijklmn", MM_INFO,
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
		    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
		    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
		    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
		"refer to manual", "ABCDEFGHIJ:abcdefghijklmn:001",
		NULL,
		"ABCDEFGHIJ:abcdefghijklmn: INFO: "
		    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
		    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
		    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
		    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
		    "TO FIX: refer to manual ABCDEFGHIJ:abcdefghijklmn:001\n"
	},
	{
		MM_OPSYS | MM_PRINT, "TEST:test", MM_HALT,
		"failed", "nothing can help me", "NOTHING",
		NULL,
		"TEST:test: HALT: failed\n"
		    "TO FIX: nothing can help me NOTHING\n"
	},
	{
		MM_OPSYS | MM_PRINT, "TEST:test", MM_WARNING,
		"failed", "nothing can help me", "NOTHING",
		NULL,
		"TEST:test: WARNING: failed\n"
		    "TO FIX: nothing can help me NOTHING\n"
	},
	{
		MM_OPSYS | MM_PRINT, "TEST:test", MM_NOSEV,
		"failed", "nothing can help me", "NOTHING",
		NULL,
		"TEST:test: failed\n"
		    "TO FIX: nothing can help me NOTHING\n"
	}
};

static char *
run_test(long classification, const char *label, int severity,
    const char *text, const char *action, const char *tag)
{
	int pip[2];
	pid_t pid, wpid;
	char *result, *p;
	size_t resultsize;
	ssize_t n;
	int status;

	if (pipe(pip) == -1)
		err(2, "pipe");
	pid = fork();
	if (pid == -1)
		err(2, "fork");
	if (pid == 0) {
		close(pip[0]);
		if (pip[1] != STDERR_FILENO &&
		    dup2(pip[1], STDERR_FILENO) == -1)
			_exit(2);
		if (fmtmsg(classification, label, severity, text, action, tag)
		    != MM_OK)
			_exit(1);
		else
			_exit(0);
	}
	close(pip[1]);
	resultsize = 1024;
	result = malloc(resultsize);
	p = result;
	while ((n = read(pip[0], p, result + resultsize - p - 1)) != 0) {
		if (n == -1) {
			if (errno == EINTR)
				continue;
			else
				err(2, "read");
		}
		p += n;
		if (result + resultsize == p - 1) {
			resultsize *= 2;
			result = realloc(result, resultsize);
			if (result == NULL)
				err(2, "realloc");
		}
	}
	if (memchr(result, '\0', p - result) != NULL) {
		free(result);
		return (NULL);
	}
	*p = '\0';
	close(pip[0]);
	while ((wpid = waitpid(pid, &status, 0)) == -1 && errno == EINTR)
		;
	if (wpid == -1)
		err(2, "waitpid");
	if (status != 0) {
		free(result);
		return (NULL);
	}
	return (result);
}

int
main(void)
{
	size_t i, n;
	int errors;
	char *result;
	struct testcase *t;

	n = sizeof(testcases) / sizeof(testcases[0]);
	errors = 0;
	printf("1..%zu\n", n);
	for (i = 0; i < n; i++) {
		t = &testcases[i];
		if (t->msgverb != NULL)
			setenv("MSGVERB", t->msgverb, 1);
		else
			unsetenv("MSGVERB");
		result = run_test(t->classification, t->label, t->severity,
		    t->text, t->action, t->tag);
		if (result != NULL && strcmp(result, t->result) == 0)
			printf("ok %zu - correct\n",
			    i + 1);
		else {
			printf("not ok %zu - %s\n",
			    i + 1, result != NULL ? "incorrect" : "failed");
			errors = 1;
		}
		free(result);
	}

	return (errors);
}