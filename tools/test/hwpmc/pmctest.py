#!/usr/bin/env python
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

# Description: A program to run a simple program against every available
# pmc counter present in a system.
#
# To use:
#
# pmctest.py -p ls > /dev/null
#
# This should result in ls being run with every available counter
# and the system should neither lock up nor panic.
#
# The default is to wait after each counter is tested.  Since the
# prompt would go to stdout you won't see it, just press return
# to continue or Ctrl-D to stop.

import sys
import subprocess
from subprocess import PIPE

# A list of strings that are not really counters, just
# name tags that are output by pmccontrol -L
notcounter = ["IAF", "IAP", "TSC", "UNC", "UCF", "UCP", "SOFT" ]

def main():

    from optparse import OptionParser
    
    parser = OptionParser()
    parser.add_option("-p", "--program", dest="program", 
                      help="program to execute")
    parser.add_option("-w", "--wait", action="store_true", dest="wait",
                      default=True, help="wait after each execution")

    (options, args) = parser.parse_args()

    if (options.program == None):
        print("specify program, such as ls, with -p/--program")
        sys.exit()
        
    p = subprocess.Popen(["pmccontrol", "-L"], stdout=PIPE)
    counters = p.communicate()[0]

    if len(counters) <= 0:
        print("no counters found")
        sys.exit()

    for counter in counters.split():
        if counter in notcounter:
            continue
        p = subprocess.Popen(["pmcstat", "-p", counter, options.program],
                             stdout=PIPE)
        result = p.communicate()[0]
        print(result)
        if (options.wait == True):
            try:
                value = input("next?")
            except EOFError:
                sys.exit()

# The canonical way to make a python module into a script.
# Remove if unnecessary.
 
if __name__ == "__main__":
    main()