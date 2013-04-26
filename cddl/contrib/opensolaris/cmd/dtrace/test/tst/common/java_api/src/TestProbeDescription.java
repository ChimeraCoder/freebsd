
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * ident	"%Z%%M%	%I%	%E% SMI"
 */

import org.opensolaris.os.dtrace.*;
import java.util.logging.*;

/**
 * Regression for 6399915 ProbeDescription single arg constructor should
 * parse probe descriptions.
 */
public class TestProbeDescription {
    public static void
    main(String[] args)
    {
	ProbeDescription p = null;
	int len = args.length;
	if (len == 0) {
	    p = new ProbeDescription("syscall:::entry");
	} else if (len == 1) {
	    p = new ProbeDescription(args[0]);
	} else if (len == 2) {
	    p = new ProbeDescription(args[0], args[1]);
	} else if (len == 3) {
	    p = new ProbeDescription(args[0], args[1], args[2]);
	} else if (len == 4) {
	    p = new ProbeDescription(args[0], args[1], args[2], args[3]);
	}
	System.out.println(p);
    }
}