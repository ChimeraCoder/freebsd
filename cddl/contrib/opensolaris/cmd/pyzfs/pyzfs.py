#! /usr/bin/python2.4 -S
# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Note, we want SIGINT (control-c) to exit the process quietly, to mimic
# the standard behavior of C programs.  The best we can do with pure
# Python is to run with -S (to disable "import site"), and start our
# program with a "try" statement.  Hopefully nobody hits ^C before our
# try statement is executed.

try:
	import site
	import gettext
	import zfs.util
	import zfs.ioctl
	import sys
	import errno

	"""This is the main script for doing zfs subcommands.  It doesn't know
	what subcommands there are, it just looks for a module zfs.<subcommand>
	that implements that subcommand."""

	_ = gettext.translation("SUNW_OST_OSCMD", "/usr/lib/locale",
	    fallback=True).gettext

	if len(sys.argv) < 2:
		sys.exit(_("missing subcommand argument"))

	zfs.ioctl.set_cmdstr(" ".join(["zfs"] + sys.argv[1:]))

	try:
		# import zfs.<subcommand>
		# subfunc =  zfs.<subcommand>.do_<subcommand>

		subcmd = sys.argv[1]
		__import__("zfs." + subcmd)
		submod = getattr(zfs, subcmd)
		subfunc = getattr(submod, "do_" + subcmd)
	except (ImportError, AttributeError):
		sys.exit(_("invalid subcommand"))

	try:
		subfunc()
	except zfs.util.ZFSError, e:
		print(e)
		sys.exit(1)

except IOError, e:
	import errno
	import sys

	if e.errno == errno.EPIPE:
		sys.exit(1)
	raise
except KeyboardInterrupt:
	import sys

	sys.exit(1)