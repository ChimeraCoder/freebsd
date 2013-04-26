#!/usr/bin/python
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

CONTROL_CHAR = 1
PRINTABLE = 2
RFC2253_QUOTE_FIRST = 4
RFC2253_QUOTE_LAST = 8
RFC2253_QUOTE = 16
RFC2253_HEX = 32

chars = []

for i in range(0, 256):
    chars.append(0);

for i in range(0, 256):
    if (i < 32 or i > 126):
        chars[i] |= CONTROL_CHAR | RFC2253_HEX;

for i in range(ord("A"), ord("Z") + 1):
    chars[i] |= PRINTABLE
for i in range(ord("a"), ord("z") + 1):
    chars[i] |= PRINTABLE
for i in range(ord("0"), ord("9") + 1):
    chars[i] |= PRINTABLE

chars[ord(' ')] |= PRINTABLE
chars[ord('+')] |= PRINTABLE
chars[ord(',')] |= PRINTABLE
chars[ord('-')] |= PRINTABLE
chars[ord('.')] |= PRINTABLE
chars[ord('/')] |= PRINTABLE
chars[ord(':')] |= PRINTABLE
chars[ord('=')] |= PRINTABLE
chars[ord('?')] |= PRINTABLE

chars[ord(' ')] |= RFC2253_QUOTE_FIRST | RFC2253_QUOTE_FIRST

chars[ord(',')] |= RFC2253_QUOTE
chars[ord('=')] |= RFC2253_QUOTE
chars[ord('+')] |= RFC2253_QUOTE
chars[ord('<')] |= RFC2253_QUOTE
chars[ord('>')] |= RFC2253_QUOTE
chars[ord('#')] |= RFC2253_QUOTE
chars[ord(';')] |= RFC2253_QUOTE

print "#define Q_CONTROL_CHAR		1"
print "#define Q_PRINTABLE		2"
print "#define Q_RFC2253_QUOTE_FIRST	4"
print "#define Q_RFC2253_QUOTE_LAST	8"
print "#define Q_RFC2253_QUOTE		16"
print "#define Q_RFC2253_HEX		32"
print ""
print "#define Q_RFC2253		(Q_RFC2253_QUOTE_FIRST|Q_RFC2253_QUOTE_LAST|Q_RFC2253_QUOTE|Q_RFC2253_HEX)"
print "\n" * 2




print "unsigned char char_map[] = {\n\t",
for x in range(0, 256):
    if (x % 8) == 0 and x != 0:
        print "\n\t",
    print "0x%(char)02x" % { 'char' : chars[x] },
    if x < 255:
        print ", ",
    else:
        print ""
print "};"