
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

/**
 * Test for bug 6399888 stop() hangs if ConsumerListener calls
 * synchronized Consumer method
 */
public class TestStopLock {
    public static void
    main(String[] args)
    {
	final Consumer consumer = new LocalConsumer();
	consumer.addConsumerListener(new ConsumerAdapter() {
	    @Override
	    public void intervalBegan(ConsumerEvent e) {
		consumer.isRunning();
	    }
	});

	try {
	    consumer.open();
	    consumer.compile("syscall:::entry { @[execname] = count(); } " +
		    "tick-101ms { printa(@); }");
	    consumer.enable();
	    consumer.go();
	    try {
		Thread.currentThread().sleep(500);
	    } catch (InterruptedException e) {
		e.printStackTrace();
		System.exit(1);
	    }
	    consumer.stop();
	    consumer.close();
	} catch (DTraceException e) {
	    e.printStackTrace();
	    System.exit(1);
	}
	System.out.println("Successful");
    }
}