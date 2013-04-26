
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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * ident	"%Z%%M%	%I%	%E% SMI"
 */
import java.util.*;
import java.util.concurrent.atomic.*;
import org.opensolaris.os.dtrace.*;

/**
 * Regression test for 6521523 aggregation drops can hang the Java
 * DTrace API.
 */
public class TestDrop {
    static final String PROGRAM =
	    "fbt:genunix::entry { @[execname, pid] = count(); }";

    static AtomicLong consumerThreadID = new AtomicLong();
    static AtomicLong getAggregateThreadID = new AtomicLong();
    static AtomicBoolean done = new AtomicBoolean();
    static int seconds;

    private static void
    startTimer()
    {
	if (seconds <= 0) {
	    return;
	}

	final Timer timer = new Timer();
	timer.schedule(new TimerTask() {
	    public void run() {
		done.set(true);
		timer.cancel();
	    }
	}, seconds * 1000L);
    }

    private static void
    sampleAggregate(Consumer consumer) throws DTraceException
    {
	while (consumer.isRunning() && !done.get()) {
	    try {
		Thread.currentThread().sleep(50);
	    } catch (InterruptedException e) {
	    }

	    consumer.getAggregate(Collections. <String> emptySet());
	}
    }

    private static void
    startAggregateThread(final Consumer consumer)
    {
	Runnable aggregateSampler = new Runnable() {
	    public void run() {
		Thread t = Thread.currentThread();
		getAggregateThreadID.set(t.getId());
		Throwable x = null;
		try {
		    sampleAggregate(consumer);
		} catch (Throwable e) {
		    x = e;
		}

		if (Thread.holdsLock(LocalConsumer.class)) {
		    if (x != null) {
			x.printStackTrace();
		    }
		    System.out.println("Lock held");
		    System.exit(1);
		} else {
		    System.out.println("Lock released");
		    consumer.close(); // blocks if lock held
		}
	    }
	};

	Thread t = new Thread(aggregateSampler, "Aggregate Sampler");
	t.start();
    }

    static void
    usage()
    {
	System.err.println("usage: java TestDrop [ seconds ]");
	System.exit(2);
    }

    public static void
    main(String[] args)
    {
	if (args.length == 1) {
	    try {
		seconds = Integer.parseInt(args[0]);
	    } catch (NumberFormatException e) {
		usage();
	    }
	} else if (args.length > 1) {
	    usage();
	}

	final Consumer consumer = new LocalConsumer() {
	    protected Thread createThread() {
		Runnable worker = new Runnable() {
		    public void run() {
			Thread t = Thread.currentThread();
			consumerThreadID.set(t.getId());
			work();
		    }
		};
		Thread t = new Thread(worker);
		return t;
	    }
	};

	consumer.addConsumerListener(new ConsumerAdapter() {
	    public void consumerStarted(ConsumerEvent e) {
		startAggregateThread(consumer);
		startTimer();
	    }
	    public void dataDropped(DropEvent e) throws ConsumerException {
		Thread t = Thread.currentThread();
		if (t.getId() == getAggregateThreadID.get()) {
		    Drop drop = e.getDrop();
		    throw new ConsumerException(drop.getDefaultMessage(),
			    drop);
		}
	    }
	});

	try {
	    consumer.open();
	    consumer.setOption(Option.aggsize, Option.kb(1));
	    consumer.setOption(Option.aggrate, Option.millis(101));
	    consumer.compile(PROGRAM);
	    consumer.enable();
	    consumer.go(new ExceptionHandler() {
		public void handleException(Throwable e) {
		    e.printStackTrace();
		}
	    });
	} catch (DTraceException e) {
	    e.printStackTrace();
	}
    }
}