
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
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

import org.opensolaris.os.dtrace.*;

/**
 * Regression for bug 6413280 lookupKernelFunction() and
 * lookupUserFunction() truncate last character.
 */
public class TestFunctionLookup {
    static final String kernelLookupProgram = "sdt:::callout-start { " +
           "@[((callout_t *)arg0)->c_func] = count(); }";
    static final String userLookupProgram = "pid$target::f2:entry { " +
           "@[arg0] = count(); }";

    public static void
    main(String[] args)
    {
	if (args.length != 1) {
	    System.err.println("usage: java TestFunctionLookup <command>");
	    System.exit(1);
	}
	String cmd = args[0];

	Consumer consumer = new LocalConsumer();
	try {
	    consumer.open();
	    consumer.compile(kernelLookupProgram);
	    consumer.enable();
	    consumer.go();
	    Aggregate a;
	    Number address;
	    String f;
	    boolean done = false;
	    for (int i = 0; (i < 20) && !done; ++i) {
		Thread.currentThread().sleep(100);
		a = consumer.getAggregate();
		for (Aggregation agg : a.getAggregations()) {
		    for (Tuple tuple : agg.asMap().keySet()) {
			address = (Number)tuple.get(0).getValue();
			if (address instanceof Integer) {
			    int addr = (Integer)address;
			    f = consumer.lookupKernelFunction(addr);
			} else {
			    long addr = (Long)address;
			    f = consumer.lookupKernelFunction(addr);
			}
			if (f.equals("genunix`cv_wakeup")) {
			    System.out.println(f);
			    done = true;
			}
		    }
		}
	    }
	    consumer.close();
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(1);
	}

	consumer = new LocalConsumer();
	try {
	    consumer.open();
	    int pid = consumer.createProcess(cmd);
	    consumer.compile(userLookupProgram);
	    consumer.enable();
	    consumer.go();
	    Thread.currentThread().sleep(500);
	    Aggregate a = consumer.getAggregate();
	    Number address;
	    String f;
	    for (Aggregation agg : a.getAggregations()) {
		for (Tuple tuple : agg.asMap().keySet()) {
		    address = (Number)tuple.get(0).getValue();
		    if (address instanceof Integer) {
			int addr = (Integer)address;
			f = consumer.lookupUserFunction(pid, addr);
		    } else {
			long addr = (Long)address;
			f = consumer.lookupUserFunction(pid, addr);
		    }
		    System.out.println(f);
		}
	    }
	    consumer.close();
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(1);
	}
    }
}