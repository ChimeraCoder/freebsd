
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

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/ioctl.h>
#include <fcntl.h>
#ifndef NONETGRAPH
#include <netgraph.h>
#endif
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#if defined(__FreeBSD__) && !defined(NOKLDLOAD)
#include <sys/linker.h>
#endif
#include <unistd.h>
#ifdef __OpenBSD__
#include <util.h>
#else
#include <libutil.h>
#endif
#include <utmpx.h>

#include "log.h"
#include "main.h"
#include "id.h"

static int uid;
static int euid;

void
ID0init()
{
  uid = getuid();
  euid = geteuid();
}

static void
ID0setuser(void)
{
  if (seteuid(uid) == -1) {
    log_Printf(LogERROR, "ID0setuser: Unable to seteuid!\n");
    AbortProgram(EX_NOPERM);
  }
}

uid_t
ID0realuid()
{
  return uid;
}

static void
ID0set0(void)
{
  if (seteuid(euid) == -1) {
    log_Printf(LogERROR, "ID0set0: Unable to seteuid!\n");
    AbortProgram(EX_NOPERM);
  }
}

int
ID0ioctl(int fd, unsigned long req, void *arg)
{
  int ret;

  ID0set0();
  ret = ioctl(fd, req, arg);
  log_Printf(LogID0, "%d = ioctl(%d, %lu, %p)\n", ret, fd, req, arg);
  ID0setuser();
  return ret;
}

int
ID0unlink(const char *name)
{
  int ret;

  ID0set0();
  ret = unlink(name);
  log_Printf(LogID0, "%d = unlink(\"%s\")\n", ret, name);
  ID0setuser();
  return ret;
}

int
ID0socket(int domain, int type, int protocol)
{
  int ret;

  ID0set0();
  ret = socket(domain, type, protocol);
  log_Printf(LogID0, "%d = socket(%d, %d, %d)\n", ret, domain, type, protocol);
  ID0setuser();
  return ret;
}

FILE *
ID0fopen(const char *path, const char *mode)
{
  FILE *ret;

  ID0set0();
  ret = fopen(path, mode);
  log_Printf(LogID0, "%p = fopen(\"%s\", \"%s\")\n", ret, path, mode);
  ID0setuser();
  return ret;
}

int
ID0open(const char *path, int flags, ...)
{
  int ret;
  va_list ap;

  va_start(ap, flags);
  ID0set0();
  ret = open(path, flags, va_arg(ap, int));
  log_Printf(LogID0, "%d = open(\"%s\", %d)\n", ret, path, flags);
  ID0setuser();
  va_end(ap);
  return ret;
}

int
ID0write(int fd, const void *data, size_t len)
{
  int ret;

  ID0set0();
  ret = write(fd, data, len);
  log_Printf(LogID0, "%d = write(%d, data, %ld)\n", ret, fd, (long)len);
  ID0setuser();
  return ret;
}

int
ID0uu_lock(const char *basettyname)
{
  int ret;

  ID0set0();
  ret = uu_lock(basettyname);
  log_Printf(LogID0, "%d = uu_lock(\"%s\")\n", ret, basettyname);
  ID0setuser();
  return ret;
}

int
ID0uu_lock_txfr(const char *basettyname, pid_t newpid)
{
  int ret;

  ID0set0();
  ret = uu_lock_txfr(basettyname, newpid);
  log_Printf(LogID0, "%d = uu_lock_txfr(\"%s\", %ld)\n", ret, basettyname,
             (long)newpid);
  ID0setuser();
  return ret;
}

int
ID0uu_unlock(const char *basettyname)
{
  int ret;

  ID0set0();
  ret = uu_unlock(basettyname);
  log_Printf(LogID0, "%d = uu_unlock(\"%s\")\n", ret, basettyname);
  ID0setuser();
  return ret;
}

void
ID0login(const struct utmpx *ut)
{
  ID0set0();
  pututxline(ut);
  log_Printf(LogID0, "pututxline(\"%.*s\", \"%.*s\", \"%.*s\", \"%.*s\")\n",
      (int)sizeof ut->ut_id, ut->ut_id,
      (int)sizeof ut->ut_user, ut->ut_user,
      (int)sizeof ut->ut_line, ut->ut_line,
      (int)sizeof ut->ut_host, ut->ut_host);
  ID0setuser();
}

void
ID0logout(const struct utmpx *ut)
{
  ID0set0();
  pututxline(ut);
  log_Printf(LogID0, "pututxline(\"%.*s\")\n",
      (int)sizeof ut->ut_id, ut->ut_id);
  ID0setuser();
}

int
ID0bind_un(int s, const struct sockaddr_un *name)
{
  int result;

  ID0set0();
  result = bind(s, (const struct sockaddr *)name, sizeof *name);
  log_Printf(LogID0, "%d = bind(%d, \"%s\", %d)\n",
            result, s, name->sun_path, (int)sizeof(*name));
  ID0setuser();
  return result;
}

int
ID0connect_un(int s, const struct sockaddr_un *name)
{
  int result;

  ID0set0();
  result = connect(s, (const struct sockaddr *)name, sizeof *name);
  log_Printf(LogID0, "%d = connect(%d, \"%s\", %d)\n",
            result, s, name->sun_path, (int)sizeof(*name));
  ID0setuser();
  return result;
}

int
ID0kill(pid_t pid, int sig)
{
  int result;

  ID0set0();
  result = kill(pid, sig);
  log_Printf(LogID0, "%d = kill(%ld, %d)\n", result, (long)pid, sig);
  ID0setuser();
  return result;
}

#if defined(__FreeBSD__) && !defined(NOKLDLOAD)
int
ID0kldload(const char *dev)
{
  int result;

  ID0set0();
  result = kldload(dev);
  log_Printf(LogID0, "%d = kldload(\"%s\")\n", result, dev);
  ID0setuser();
  return result;
}
#endif

#ifndef NONETGRAPH
int
ID0NgMkSockNode(const char *name, int *cs, int *ds)
{
  int result;

  ID0set0();
  result = NgMkSockNode(name, cs, ds);
  log_Printf(LogID0, "%d = NgMkSockNode(\"%s\", &cs, &ds)\n",
             result, name ? name : "");
  ID0setuser();
  return result;
}
#endif