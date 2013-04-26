
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

import org.opensolaris.os.dtrace.*;

/**
 * Regression for 6506495 -DJAVA_DTRACE_MAX_CONSUMERS=N for any N &lt; 8
 * is treated as if it were 8.
 */
public class TestMaxConsumers {
    static final String MAX_CONSUMERS_PROPERTY_NAME =
	    "JAVA_DTRACE_MAX_CONSUMERS";

    static Integer
    getIntegerProperty(String name)
    {
	Integer value = null;
	String property = System.getProperty(name);
	if (property != null && property.length() != 0) {
	    try {
		value = Integer.parseInt(property);
	    } catch (NumberFormatException e) {
		e.printStackTrace();
	    }
	}
	return value;
    }

    public static void
    main(String[] args)
    {
	Integer property = getIntegerProperty(MAX_CONSUMERS_PROPERTY_NAME);
	int max = (property == null ? 0 : property);
	int n = (property == null ? 11 : (max < 1 ? 1 : max));

	Consumer[] consumers = new Consumer[n];
	try {
	    for (int i = 0; i < n; ++i) {
		consumers[i] = new LocalConsumer();
		consumers[i].open();
	    }
	    for (int i = 0; i < n; ++i) {
		consumers[i].close();
	    }
	    for (int i = 0; i < n; ++i) {
		consumers[i] = new LocalConsumer();
		consumers[i].open();
	    }
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(1);
	}

	try {
	    Consumer consumer = new LocalConsumer();
	    consumer.open();
	    if (max > 0) {
		System.out.println("Error: " + (max + 1) + " > " +
			MAX_CONSUMERS_PROPERTY_NAME);
	    } else {
		System.out.println("Success");
	    }
	    consumer.close();
	} catch (Exception e) {
	    System.out.println("Success");
	} finally {
	    for (int i = 0; i < n; ++i) {
		consumers[i].close();
	    }
	}
    }
}