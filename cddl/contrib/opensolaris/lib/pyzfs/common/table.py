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

import zfs.util

class Table:
	__slots__ = "fields", "rjustfields", "maxfieldlen", "lines"
	__repr__ = zfs.util.default_repr

	def __init__(self, fields, rjustfields=()):
		# XXX maybe have a defaults, too?
		self.fields = fields
		self.rjustfields = rjustfields
		self.maxfieldlen = dict.fromkeys(fields, 0)
		self.lines = list()
	
	def __updatemax(self, k, v):
		self.maxfieldlen[k] = max(self.maxfieldlen.get(k, None), v)

	def addline(self, sortkey, values):
		"""values is a dict from field name to value"""

		va = list()
		for f in self.fields:
			v = str(values[f])
			va.append(v)
			self.__updatemax(f, len(v))
		self.lines.append((sortkey, va))

	def printme(self, headers=True):
		if headers:
			d = dict([(f, f.upper()) for f in self.fields])
			self.addline(None, d)

		self.lines.sort()
		for (k, va) in self.lines:
			line = str()
			for i in range(len(self.fields)):
				if not headers:
					line += va[i]
					line += "\t"
				else:
					if self.fields[i] in self.rjustfields:
						fmt = "%*s  "
					else:
						fmt = "%-*s  "
					mfl = self.maxfieldlen[self.fields[i]]
					line += fmt % (mfl, va[i])
			print(line)