
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <efi.h>
#include <efilib.h>

#include <time.h>
#include <sys/time.h>

/*
// Accurate only for the past couple of centuries;
// that will probably do.
//
// (#defines From FreeBSD 3.2 lib/libc/stdtime/tzfile.h)
*/

#define isleap(y)	(((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define SECSPERHOUR ( 60*60 )
#define SECSPERDAY	(24 * SECSPERHOUR)

time_t
efi_time(EFI_TIME *ETime)
{
    /*
    //  These arrays give the cumulative number of days up to the first of the
    //  month number used as the index (1 -> 12) for regular and leap years.
    //  The value at index 13 is for the whole year.
    */
    static time_t CumulativeDays[2][14] = {
    {0,
     0,
     31,
     31 + 28,
     31 + 28 + 31,
     31 + 28 + 31 + 30,
     31 + 28 + 31 + 30 + 31,
     31 + 28 + 31 + 30 + 31 + 30,
     31 + 28 + 31 + 30 + 31 + 30 + 31,
     31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
     31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
     31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
     31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
     31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31 },
    {0,
     0,
     31,
     31 + 29,
     31 + 29 + 31,
     31 + 29 + 31 + 30,
     31 + 29 + 31 + 30 + 31,
     31 + 29 + 31 + 30 + 31 + 30,
     31 + 29 + 31 + 30 + 31 + 30 + 31,
     31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
     31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
     31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
     31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
     31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31 }};

    time_t  UTime; 
    int     Year;

    /*
    //  Do a santity check
    */
    if ( ETime->Year  <  1998 || ETime->Year   > 2099 ||
    	 ETime->Month ==    0 || ETime->Month  >   12 ||
    	 ETime->Day   ==    0 || ETime->Month  >   31 ||
    	                         ETime->Hour   >   23 ||
    	                         ETime->Minute >   59 ||
    	                         ETime->Second >   59 ||
    	 ETime->TimeZone  < -1440                     ||
    	 (ETime->TimeZone >  1440 && ETime->TimeZone != 2047) ) {
    	return (0);
    }

    /*
    // Years
    */
    UTime = 0;
    for (Year = 1970; Year != ETime->Year; ++Year) {
        UTime += (CumulativeDays[isleap(Year)][13] * SECSPERDAY);
    }

    /*
    // UTime should now be set to 00:00:00 on Jan 1 of the file's year.
    //
    // Months  
    */
    UTime += (CumulativeDays[isleap(ETime->Year)][ETime->Month] * SECSPERDAY);

    /*
    // UTime should now be set to 00:00:00 on the first of the file's month and year
    //
    // Days -- Don't count the file's day
    */
    UTime += (((ETime->Day > 0) ? ETime->Day-1:0) * SECSPERDAY);

    /*
    // Hours
    */
    UTime += (ETime->Hour * SECSPERHOUR);

    /*
    // Minutes
    */
    UTime += (ETime->Minute * 60);

    /*
    // Seconds
    */
    UTime += ETime->Second;

    /*
    //  EFI time is repored in local time.  Adjust for any time zone offset to
    //  get true UT
    */
    if ( ETime->TimeZone != EFI_UNSPECIFIED_TIMEZONE ) {
    	/*
    	//  TimeZone is kept in minues...
    	*/
    	UTime += (ETime->TimeZone * 60);
    }
    
    return UTime;
}

int
EFI_GetTimeOfDay(
	OUT struct timeval *tp,
	OUT struct timezone *tzp
	)
{
	EFI_TIME		EfiTime;
	EFI_TIME_CAPABILITIES	Capabilities;
	EFI_STATUS		Status;

	/*
	//  Get time from EFI
	*/

	Status = RS->GetTime(&EfiTime, &Capabilities);
	if (EFI_ERROR(Status))
		return (-1);

	/*
	//  Convert to UNIX time (ie seconds since the epoch
	*/

	tp->tv_sec  = efi_time( &EfiTime );
	tp->tv_usec = 0; /* EfiTime.Nanosecond * 1000; */

	/*
	//  Do something with the timezone if needed
	*/

	if (tzp) {
		tzp->tz_minuteswest =
			EfiTime.TimeZone == EFI_UNSPECIFIED_TIMEZONE ? 0 : EfiTime.TimeZone;
		/*
		//  This isn't quit right since it doesn't deal with EFI_TIME_IN_DAYLIGHT
		*/
		tzp->tz_dsttime =
			EfiTime.Daylight & EFI_TIME_ADJUST_DAYLIGHT ? 1 : 0;
	}

	return (0);
}

time_t
time(time_t *tloc)
{
	struct timeval tv;
	EFI_GetTimeOfDay(&tv, 0);
	
	if (tloc)
		*tloc = tv.tv_sec;
	return tv.tv_sec;
}

time_t
getsecs()
{
    return time(0);
}