#!/bin/sh
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

echo '%!'
echo '/beastie {'
cat beastie.eps
echo '} def'
cat FreeBSD.pfa

echo '

/mm {25.4 div 72 mul} def
/FreeBSD findfont 120 scalefont setfont
/center 210 mm 2 div def
/top 297 mm def
/cshow { dup stringwidth pop 2 div neg 0 rmoveto show } def

% "FreeBSD" across the top.
% 691 is "top - height of caps - (left - X("F"))"
center 691 moveto
(FreeBSD) cshow

% Put beastie in the center
/sc 1.25 def
center 125 moveto 
384 sc mul 2 div neg 0 rmoveto
gsave currentpoint translate sc sc scale beastie grestore

% A box for the bottom text
gsave
10 30 moveto
210 mm 20 sub 0 rlineto
0 70 rlineto
210 mm 20 sub neg 0 rlineto
closepath
.7 .7 .9 setrgbcolor
fill
grestore

% Bottom text
center 90 moveto 
/FreeBSD findfont 50 scalefont setfont

center 50 moveto 
(http://www.FreeBSD.org) cshow

% Do not forget Kirks copyright string.
10 105 moveto 
/Times-Roman findfont 8 scalefont setfont
(BSD Daemon ) show
/Symbol findfont 8 scalefont setfont
(\343 ) show
/Times-Roman findfont 8 scalefont setfont
(Copyright 1988 by Marshall Kirk McKusick.  All Rights Reserved.) show

showpage
'