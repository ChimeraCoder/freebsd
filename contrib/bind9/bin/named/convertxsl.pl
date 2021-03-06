#!/usr/bin/env perl
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

# $Id: convertxsl.pl,v 1.14 2008/07/17 23:43:26 jinmei Exp $

use strict;
use warnings;

my $rev = '$Id: convertxsl.pl,v 1.14 2008/07/17 23:43:26 jinmei Exp $';
$rev =~ s/\$//g;
$rev =~ s/,v//g;
$rev =~ s/Id: //;

my $xsl = "unknown";
my $lines = '';

while (<>) {
    chomp;
    # pickout the id for comment.
    $xsl = $_ if (/<!-- .Id:.* -->/);
    # convert Id string to a form not recognisable by cvs.
    $_ =~ s/<!-- .Id:(.*). -->/<!-- \\045Id: $1\\045 -->/;
    s/[\ \t]+/ /g;
    s/\>\ \</\>\</g;
    s/\"/\\\"/g;
    s/^/\t\"/;
    s/$/\\n\"/;
    if ($lines eq "") {
	    $lines .= $_;
    } else {
	    $lines .= "\n" . $_;
    }
}

$xsl =~ s/\$//g;
$xsl =~ s/<!-- Id: //;
$xsl =~ s/ -->.*//;
$xsl =~ s/,v//;

print "/*\n * Generated by $rev \n * From $xsl\n */\n";
print 'static char xslmsg[] =',"\n";
print $lines;

print ';', "\n";