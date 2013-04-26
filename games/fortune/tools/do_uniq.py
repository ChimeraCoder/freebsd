#!/usr/local/bin/python
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
#
# $FreeBSD$
#
# an aggressive little script for trimming duplicate cookies
import re, sys

wordlist = [
    'hadnot',
    'donot', 'hadnt',
    'dont', 'have', 'more', 'will', 'your',
    'and', 'are', 'had', 'the', 'you',
    'am', 'an', 'is', 'll', 've', 'we',
    'a', 'd', 'i', 'm', 's',
]

def hash(fortune):
    f = fortune
    f = f.lower()
    f = re.sub('[\W_]', '', f)
    for word in wordlist:
        f = re.sub(word, '', f)
#    f = re.sub('[aeiouy]', '', f)
#    f = re.sub('[^aeiouy]', '', f)
    f = f[:30]
#    f = f[-30:]
    return f

def edit(datfile):
    dups = {}
    fortunes = []
    fortune = ""
    for line in file(datfile):
        if line == "%\n":
            key = hash(fortune)
            if key not in dups:
                dups[key] = []
            dups[key].append(fortune)
            fortunes.append(fortune)
            fortune = ""
        else:
            fortune += line
    for key in list(dups.keys()):
        if len(dups[key]) == 1:
            del dups[key]
    o = file(datfile + '~', "w")
    for fortune in fortunes:
        key = hash(fortune)
        if key in dups:
            print('\n' * 50)
            for f in dups[key]:
                if f != fortune:
                    print(f, '%')
            print(fortune, '%')
            if input("Remove last fortune? ") == 'y':
                del dups[key]
                continue
        o.write(fortune + "%\n")
    o.close()

assert len(sys.argv) == 2
edit(sys.argv[1])