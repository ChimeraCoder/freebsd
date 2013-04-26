
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/clock.h>

#define DAY	(24 * 60 * 60)	/* Length of day in seconds */
#define YEAR	365		/* Length of normal year */
#define LYC	(4 * YEAR + 1)	/* Length of 4 year leap-year cycle */
#define T1980	(10 * 365 + 2)	/* Days from 1970 to 1980 */

/* End of month is N days from start of (normal) year */
#define JAN	31
#define FEB	(JAN + 28)
#define MAR	(FEB + 31)
#define APR	(MAR + 30)
#define MAY	(APR + 31)
#define JUN	(MAY + 30)
#define JUL	(JUN + 31)
#define AUG	(JUL + 31)
#define SEP	(AUG + 30)
#define OCT	(SEP + 31)
#define NOV	(OCT + 30)
#define DEC	(NOV + 31)

/* Table of months in a 4 year leap-year cycle */

#define ENC(y,m)	(((y) << 9) | ((m) << 5))

static const struct {
	uint16_t	days;	/* month start in days relative to cycle */
	uint16_t	coded;	/* encoded year + month information */
} mtab[48] = {
	{   0 + 0 * YEAR,     ENC(0, 1)  },

	{ JAN + 0 * YEAR,     ENC(0, 2)  }, { FEB + 0 * YEAR + 1, ENC(0, 3)  },
	{ MAR + 0 * YEAR + 1, ENC(0, 4)  }, { APR + 0 * YEAR + 1, ENC(0, 5)  },
	{ MAY + 0 * YEAR + 1, ENC(0, 6)  }, { JUN + 0 * YEAR + 1, ENC(0, 7)  },
	{ JUL + 0 * YEAR + 1, ENC(0, 8)  }, { AUG + 0 * YEAR + 1, ENC(0, 9)  },
	{ SEP + 0 * YEAR + 1, ENC(0, 10) }, { OCT + 0 * YEAR + 1, ENC(0, 11) },
	{ NOV + 0 * YEAR + 1, ENC(0, 12) }, { DEC + 0 * YEAR + 1, ENC(1, 1)  },

	{ JAN + 1 * YEAR + 1, ENC(1, 2)  }, { FEB + 1 * YEAR + 1, ENC(1, 3)  },
	{ MAR + 1 * YEAR + 1, ENC(1, 4)  }, { APR + 1 * YEAR + 1, ENC(1, 5)  },
	{ MAY + 1 * YEAR + 1, ENC(1, 6)  }, { JUN + 1 * YEAR + 1, ENC(1, 7)  },
	{ JUL + 1 * YEAR + 1, ENC(1, 8)  }, { AUG + 1 * YEAR + 1, ENC(1, 9)  },
	{ SEP + 1 * YEAR + 1, ENC(1, 10) }, { OCT + 1 * YEAR + 1, ENC(1, 11) },
	{ NOV + 1 * YEAR + 1, ENC(1, 12) }, { DEC + 1 * YEAR + 1, ENC(2, 1)  },

	{ JAN + 2 * YEAR + 1, ENC(2, 2)  }, { FEB + 2 * YEAR + 1, ENC(2, 3)  },
	{ MAR + 2 * YEAR + 1, ENC(2, 4)  }, { APR + 2 * YEAR + 1, ENC(2, 5)  },
	{ MAY + 2 * YEAR + 1, ENC(2, 6)  }, { JUN + 2 * YEAR + 1, ENC(2, 7)  },
	{ JUL + 2 * YEAR + 1, ENC(2, 8)  }, { AUG + 2 * YEAR + 1, ENC(2, 9)  },
	{ SEP + 2 * YEAR + 1, ENC(2, 10) }, { OCT + 2 * YEAR + 1, ENC(2, 11) },
	{ NOV + 2 * YEAR + 1, ENC(2, 12) }, { DEC + 2 * YEAR + 1, ENC(3, 1)  },

	{ JAN + 3 * YEAR + 1, ENC(3, 2)  }, { FEB + 3 * YEAR + 1, ENC(3, 3)  },
	{ MAR + 3 * YEAR + 1, ENC(3, 4)  }, { APR + 3 * YEAR + 1, ENC(3, 5)  },
	{ MAY + 3 * YEAR + 1, ENC(3, 6)  }, { JUN + 3 * YEAR + 1, ENC(3, 7)  },
	{ JUL + 3 * YEAR + 1, ENC(3, 8)  }, { AUG + 3 * YEAR + 1, ENC(3, 9)  },
	{ SEP + 3 * YEAR + 1, ENC(3, 10) }, { OCT + 3 * YEAR + 1, ENC(3, 11) },
	{ NOV + 3 * YEAR + 1, ENC(3, 12) }
};


void
timespec2fattime(struct timespec *tsp, int utc, uint16_t *ddp, uint16_t *dtp, uint8_t *dhp)
{
	time_t t1;
	unsigned t2, l, m;

	t1 = tsp->tv_sec;
	if (!utc)
		t1 -= utc_offset();

	if (dhp != NULL)
		*dhp = (tsp->tv_sec & 1) * 100 + tsp->tv_nsec / 10000000;
	if (dtp != NULL) {
		*dtp = (t1 / 2) % 30;
		*dtp |= ((t1 / 60) % 60) << 5;
		*dtp |= ((t1 / 3600) % 24) << 11;
	}
	if (ddp != NULL) {
		t2 = t1 / DAY;
		if (t2 < T1980) {
			/* Impossible date, truncate to 1980-01-01 */
			*ddp = 0x0021;
		} else {
			t2 -= T1980;

			/*
			 * 2100 is not a leap year.
			 * XXX: a 32 bit time_t can not get us here.
			 */
			if (t2 >= ((2100 - 1980) / 4 * LYC + FEB))
				t2++;

			/* Account for full leapyear cycles */
			l = t2 / LYC;
			*ddp = (l * 4) << 9;
			t2 -= l * LYC;

			/* Find approximate table entry */
			m = t2 / 32;

			/* Find correct table entry */
			while (m < 47 && mtab[m + 1].days <= t2)
				m++;

			/* Get year + month from the table */
			*ddp += mtab[m].coded;

			/* And apply the day in the month */
			t2 -= mtab[m].days - 1;
			*ddp |= t2;
		}
	}
}

/*
 * Table indexed by the bottom two bits of year + four bits of the month
 * from the FAT timestamp, returning number of days into 4 year long
 * leap-year cycle
 */

#define DCOD(m, y, l)	((m) + YEAR * (y) + (l))
static const uint16_t daytab[64] = {
	0, 		 DCOD(  0, 0, 0), DCOD(JAN, 0, 0), DCOD(FEB, 0, 1),
	DCOD(MAR, 0, 1), DCOD(APR, 0, 1), DCOD(MAY, 0, 1), DCOD(JUN, 0, 1),
	DCOD(JUL, 0, 1), DCOD(AUG, 0, 1), DCOD(SEP, 0, 1), DCOD(OCT, 0, 1),
	DCOD(NOV, 0, 1), DCOD(DEC, 0, 1), 0,               0,
	0, 		 DCOD(  0, 1, 1), DCOD(JAN, 1, 1), DCOD(FEB, 1, 1),
	DCOD(MAR, 1, 1), DCOD(APR, 1, 1), DCOD(MAY, 1, 1), DCOD(JUN, 1, 1),
	DCOD(JUL, 1, 1), DCOD(AUG, 1, 1), DCOD(SEP, 1, 1), DCOD(OCT, 1, 1),
	DCOD(NOV, 1, 1), DCOD(DEC, 1, 1), 0,               0,
	0,		 DCOD(  0, 2, 1), DCOD(JAN, 2, 1), DCOD(FEB, 2, 1),
	DCOD(MAR, 2, 1), DCOD(APR, 2, 1), DCOD(MAY, 2, 1), DCOD(JUN, 2, 1),
	DCOD(JUL, 2, 1), DCOD(AUG, 2, 1), DCOD(SEP, 2, 1), DCOD(OCT, 2, 1),
	DCOD(NOV, 2, 1), DCOD(DEC, 2, 1), 0,               0,
	0,		 DCOD(  0, 3, 1), DCOD(JAN, 3, 1), DCOD(FEB, 3, 1),
	DCOD(MAR, 3, 1), DCOD(APR, 3, 1), DCOD(MAY, 3, 1), DCOD(JUN, 3, 1),
	DCOD(JUL, 3, 1), DCOD(AUG, 3, 1), DCOD(SEP, 3, 1), DCOD(OCT, 3, 1),
	DCOD(NOV, 3, 1), DCOD(DEC, 3, 1), 0,               0
};

void
fattime2timespec(unsigned dd, unsigned dt, unsigned dh, int utc, struct timespec *tsp)
{
	unsigned day;

	/* Unpack time fields */
	tsp->tv_sec = (dt & 0x1f) << 1;
	tsp->tv_sec += ((dt & 0x7e0) >> 5) * 60;
	tsp->tv_sec += ((dt & 0xf800) >> 11) * 3600;
	tsp->tv_sec += dh / 100;
	tsp->tv_nsec = (dh % 100) * 10000000;

	/* Day of month */
	day = (dd & 0x1f) - 1;

	/* Full leap-year cycles */
	day += LYC * ((dd >> 11) & 0x1f);

	/* Month offset from leap-year cycle */
	day += daytab[(dd >> 5) & 0x3f];

	/*
	 * 2100 is not a leap year.
	 * XXX: a 32 bit time_t can not get us here.
	 */
	if (day >= ((2100 - 1980) / 4 * LYC + FEB))
		day--;

	/* Align with time_t epoch */
	day += T1980;

	tsp->tv_sec += DAY * day;
	if (!utc)
		tsp->tv_sec += utc_offset();
}

#ifdef TEST_DRIVER

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int
main(int argc __unused, char **argv __unused)
{
	int i;
	struct timespec ts;
	struct tm tm;
	double a;
	uint16_t d, t;
	uint8_t p;
	char buf[100];

	for (i = 0; i < 10000; i++) {
		do {
			ts.tv_sec = random();
		} while (ts.tv_sec < T1980 * 86400);
		ts.tv_nsec = random() % 1000000000;

		printf("%10d.%03ld -- ", ts.tv_sec, ts.tv_nsec / 1000000);

		gmtime_r(&ts.tv_sec, &tm);
		strftime(buf, sizeof buf, "%Y %m %d %H %M %S", &tm);
		printf("%s -- ", buf);

		a = ts.tv_sec + ts.tv_nsec * 1e-9;
		d = t = p = 0;
		timet2fattime(&ts, &d, &t, &p);
		printf("%04x %04x %02x -- ", d, t, p);
		printf("%3d %02d %02d %02d %02d %02d -- ",
		    ((d >> 9)  & 0x7f) + 1980,
		    (d >> 5)  & 0x0f,
		    (d >> 0)  & 0x1f,
		    (t >> 11) & 0x1f,
		    (t >> 5)  & 0x3f,
		    ((t >> 0)  & 0x1f) * 2);

		ts.tv_sec = ts.tv_nsec = 0;
		fattime2timet(d, t, p, &ts);
		printf("%10d.%03ld == ", ts.tv_sec, ts.tv_nsec / 1000000);
		gmtime_r(&ts.tv_sec, &tm);
		strftime(buf, sizeof buf, "%Y %m %d %H %M %S", &tm);
		printf("%s -- ", buf);
		a -= ts.tv_sec + ts.tv_nsec * 1e-9;
		printf("%.3f", a);
		printf("\n");
	}
	return (0);
}

#endif /* TEST_DRIVER */