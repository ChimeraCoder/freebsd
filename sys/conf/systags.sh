#! /bin/sh
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

rm -f tags tags.tmp tags.cfiles tags.sfiles tags.hfiles
sed -e "s, machine/, ../../include/,g" \
	-e 's,[a-z][^/    ]*/\.\./,,g' .depend | awk '{
		for (i = 1; i <= NF; ++i) {
			t = substr($i, length($i) - 1)
			if (t == ".c")
				cfiles[$i] = 1;
			else if (t == ".h")
				hfiles[$i] = 1;
			else if (t == ".s")
				sfiles[$i] = 1;
		}
	};
	END {
		for (i in cfiles)
			print i > "tags.cfiles";
		for (i in sfiles)
			print i > "tags.sfiles";
		for (i in hfiles)
			print i > "tags.hfiles";
	}'

ctags -t -d -w `cat tags.cfiles tags.hfiles tags.sfiles`
egrep "^ENTRY\(.*\)|^ALTENTRY\(.*\)" `cat tags.sfiles` | \
    sed "s;\([^:]*\):\([^(]*\)(\([^, )]*\)\(.*\);\3	\1	/^\2(\3\4$/;" >> tags

mv tags tags.tmp
sort -u tags.tmp > tags
rm tags.tmp tags.cfiles tags.sfiles tags.hfiles