#!/usr/bin/perl
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

# $Id: format-options.pl,v 1.5 2007/09/24 04:21:59 marka Exp $

print <<END;

This is a summary of the named.conf options supported by 
this version of BIND 9.

END

# Break long lines
while (<>) {
	chomp;
	s/\t/        /g;
	my $line = $_;
	m!^( *)!;
	my $indent = $1;
	my $comment = "";
	if ( $line =~ m!//.*! ) {
		$comment = $&;
		$line =~ s!//.*!!;
	}
	my $start = "";
	while (length($line) >= 79 - length($comment)) {
		$_ = $line;
		# this makes sure that the comment has something in front of it
		$len = 75 - length($comment);
		m!^(.{0,$len}) (.*)$!;
		$start = $start.$1."\n";
		$line = $indent."    ".$2;
	}
	print $start.$line.$comment."\n";
}