#! /usr/bin/python2.6
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

"""This module implements the "zfs holds" subcommand.
The only public interface is the zfs.holds.do_holds() function."""

import optparse
import sys
import errno
import time
import zfs.util
import zfs.dataset
import zfs.table

_ = zfs.util._

def do_holds():
	"""Implements the "zfs holds" subcommand."""
	def usage(msg=None):
		parser.print_help()
		if msg:
			print
			parser.exit("zfs: error: " + msg)
		else:
			parser.exit()

	u = _("""holds [-r] <snapshot> ...""")

	parser = optparse.OptionParser(usage=u, prog="zfs")

	parser.add_option("-r", action="store_true", dest="recursive",
	    help=_("list holds recursively"))

	(options, args) = parser.parse_args(sys.argv[2:])

	if len(args) < 1:
		usage(_("missing snapshot argument"))

	fields = ("name", "tag", "timestamp")
	rjustfields = ()
	printing = False 
	gotone = False
	t = zfs.table.Table(fields, rjustfields) 
	for ds in zfs.dataset.snapshots_fromcmdline(args, options.recursive):
		gotone = True
		for tag, tm in ds.get_holds().iteritems():
			val = {"name": ds.name, "tag": tag,
			    "timestamp": time.ctime(tm)}
			t.addline(ds.name, val)
			printing = True
	if printing:
		t.printme()
	elif not gotone:
		raise zfs.util.ZFSError(errno.ENOENT, _("no matching datasets"))