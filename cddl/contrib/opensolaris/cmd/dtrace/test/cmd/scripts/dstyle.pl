#!/usr/local/bin/perl
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
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

require 5.8.4;

$PNAME = $0;
$PNAME =~ s:.*/::;
$USAGE = "Usage: $PNAME [file ...]\n";
$errs = 0;

sub err
{
	my($msg) = @_;

	print "$file: $lineno: $msg\n";
	$errs++;
}

sub dstyle
{
	open(FILE, "$file");
	$lineno = 0;
	$inclause = 0;
	$skipnext = 0;

	while (<FILE>) {
		$lineno++;

		chop;

		if ($skipnext) {
			$skipnext = 0;
			next;
		}

		#
		# Amazingly, some ident strings are longer than 80 characters!
		#
		if (/^#pragma ident/) {
			next;
		}

		#
		# The algorithm to calculate line length from cstyle.
		#
		$line = $_;
		if ($line =~ tr/\t/\t/ * 7 + length($line) > 80) {
			# yes, there is a chance.
			# replace tabs with spaces and check again.
			$eline = $line;
			1 while $eline =~
			    s/\t+/' ' x (length($&) * 8 - length($`) % 8)/e;

			if (length($eline) > 80) {
				err "line > 80 characters";
			}
		}

		if (/\/\*DSTYLED\*\//) {
			$skipnext = 1;
			next;
		}

		if (/^#pragma/) {
			next;
		}

		if (/^#include/) {
			next;
		}

		#
		# Before we do any more analysis, we want to prune out any
		# quoted strings.  This is a bit tricky because we need
		# to be careful of backslashed quotes within quoted strings.
		# I'm sure there is a very crafty way to do this with a
		# single regular expression, but that will have to wait for
		# somone with better regex juju that I; we do this by first
		# eliminating the backslashed quotes, and then eliminating
		# whatever quoted strings are left.  Note that we eliminate
		# the string by replacing it with "quotedstr"; this is to
		# allow lines to end with a quoted string.  (If we simply
		# eliminated the quoted string, dstyle might complain about
		# the line ending in a space or tab.)
		# 
		s/\\\"//g;
		s/\"[^\"]*\"/quotedstr/g;

		if (/[ \t]$/) {
			err "space or tab at end of line";
		}

		if (/^[\t]+[ ]+[\t]+/) {
			err "spaces between tabs";
		}

		if (/^[\t]* \*/) {
			next;
		}

		if (/^        /) {
			err "indented by spaces not tabs";
		}

		if (/^{}$/) {
			next;
		}

		if (!/^enum/ && !/^\t*struct/ && !/^\t*union/ && !/^typedef/ &&
		    !/^translator/ && !/^provider/) {
			if (/[\w\s]+{/) {
				err "left brace not on its own line";
			}

			if (/{[\w\s]+/) {
				err "left brace not on its own line";
			}
		}

		if (!/;$/) {
			if (/[\w\s]+}/) {
				err "right brace not on its own line";
			}

			if (/}[\w\s]+/) {
				err "right brace not on its own line";
			}
		}

		if (/^}/) {
			$inclause = 0;
		}

		if (!$inclause && /^[\w ]+\//) {
			err "predicate not at beginning of line";
		}

		if (!$inclause && /^\/[ \t]+\w/) {
			err "space between '/' and expression in predicate";
		}

		if (!$inclause && /\w[ \t]+\/$/) {
			err "space between expression and '/' in predicate";
		}

		if (!$inclause && /\s,/) {
			err "space before comma in probe description";
		}

		if (!$inclause && /\w,[\w\s]/ && !/;$/) {
			if (!/extern/ && !/\(/ && !/inline/) {
				err "multiple probe descriptions on same line";
			}
		}

		if ($inclause && /sizeof\(/) {
			err "missing space after sizeof";
		}

		if ($inclause && /^[\w ]/) {
			err "line doesn't begin with a tab";
		}

		if ($inclause && /,[\w]/) {
			err "comma without trailing space";
		}

		if (/\w&&/ || /&&\w/ || /\w\|\|/ || /\|\|\w/) {
			err "logical operator not set off with spaces";
		}

		#
		# We want to catch "i<0" variants, but we don't want to
		# erroneously flag translators.
		#
		if (!/\w<\w+>\(/) {
			if (/\w>/ || / >\w/ || /\w</ || /<\w/) {
				err "comparison operator not set " . 
				    "off with spaces";
			}
		}

		if (/\w==/ || /==\w/ || /\w<=/ || />=\w/ || /\w!=/ || /!=\w/) {
			err "comparison operator not set off with spaces";
		}

		if (/\w=/ || /=\w/) {
			err "assignment operator not set off with spaces";
		}

		if (/^{/) {
			$inclause = 1;
		}
        }
}

foreach $arg (@ARGV) {
	if (-f $arg) {
		push(@files, $arg);
	} else {
		die "$PNAME: $arg is not a valid file\n";
	}
}

die $USAGE if (scalar(@files) == 0);

foreach $file (@files) {
	dstyle($file);
}

exit($errs != 0);