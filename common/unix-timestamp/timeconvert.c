#include <stdint.h>
#include <time.h>
#include "timeconvert.h"

static void  civil_from_days(uint64_t *year, uint64_t *month, uint64_t *day, uint64_t ts_day);
static uint64_t weekday_from_days(int64_t z);

/**
 * Obtains an ASCII representation of the time
 * @param timeptr 		as a pointer to a time struct
 * @return 				ASCII character string
 */
char *get_timestring(const struct tm * timeptr)
{
	return asctime(timeptr);
} // end


/**
 * Compute the number of days from 1 Jan 1970
 * @param y 		current year
 * @param m 		current month
 * @param d 		current day
 * @return
 */
uint64_t days_from_civil(uint64_t y, uint64_t m, uint64_t d)
{
	y -= (m <= 2) ? 1 : 0;
	uint64_t era = y / 400;
	uint64_t yoe = (y - era * 400);      							// [0, 399]
	uint64_t doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;  		// [0, 365]
	uint64_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
	return era * 146097 + doe - 719468;
} // end


/**
 * Obtain the number of seconds since 1 January 1970
 * @param y
 * @param m
 * @param d
 * @param h
 * @param minute
 * @param sec
 * @return
 */
uint64_t sec_from_civil(uint64_t y, uint64_t m, uint64_t d, uint64_t h, uint64_t minute, uint64_t sec)
{
	uint64_t sec_current_day = h*3600 + minute*60 + sec;
	uint64_t days = days_from_civil(y, m, d);
	uint64_t sec_days = (days > 0) ? 86400*days : 0;
	uint64_t output = sec_current_day + sec_days;
	return output;
} // end


/**
 * Given a ctime struct, convert to seconds since 1 Jan 1970
 * @param times
 * @return
 */
uint64_t epoch_from_time(const struct tm *times)
{
	uint64_t y, m, d, h, minute, sec;
	y = times->tm_year + 1900;		// year is actual year (i.e. 2021)
	m = times->tm_mon + 1;  		// month starts at 1
	d = times->tm_mday;
	h = times->tm_hour;
	minute = times->tm_min;
	sec = times->tm_sec;
	uint64_t out = sec_from_civil(y, m, d, h, minute, sec);
	return out;
} // end


/**
 * Obtain year, month and day from a day number since 1 January 1970
 * @param year
 * @param month
 * @param day
 * @param ts_day
 * REFERENCE:  https://stackoverflow.com/questions/7960318/math-to-convert-seconds-since-1970-into-date-and-vice-versa
 */
void  civil_from_days(uint64_t *year, uint64_t *month, uint64_t *day, const uint64_t ts_day)
{
	uint64_t z = ts_day;
	z += 719468;
	uint64_t era = z / 146097;
	uint64_t doe = (z - era * 146097);          					 // [0, 146096]
	uint64_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
	uint64_t y = yoe + era * 400;
	uint64_t doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
	uint64_t mp = (5*doy + 2)/153;                                   // [0, 11]
	uint64_t d = doy - (153*mp+2)/5 + 1;                             // [1, 31]
	uint64_t m = mp + (mp < 10 ? 3 : -9);                            // [1, 12]

	*year = y + ((m <= 2) ? 1 : 0);
	*month = m;
	*day = d;
} // end


/**
 * Weekday from days
 * @param z 			day numbers since 1 January 1970
 * @return
 * REFERENCE:  https://stackoverflow.com/questions/7960318/math-to-convert-seconds-since-1970-into-date-and-vice-versa
 */
uint64_t weekday_from_days(const int64_t z)
{
	return (z+4) % 7;
} // end


/**
 * Converts a UNIX timestamp to a C time in UTC
 * @param time		as the C time struct
 * @param ts		as the UNIX timestamp
 * REFERENCES: https://stackoverflow.com/questions/7960318/math-to-convert-seconds-since-1970-into-date-and-vice-versa
 * https://github.com/sidsingh78/EPOCH-to-time-date-converter/blob/master/epoch_conv.c
 */
void epoch_to_time(struct tm *times, const uint64_t ts)
{
	const uint64_t seconds_in_day = 86400;
	uint64_t epoch = ts;
	uint64_t second = epoch%60;
	epoch /= 60;
	uint64_t minute = epoch%60;
	epoch /= 60;
	uint64_t hour  = epoch%24;

	uint64_t year, month, day;
	uint64_t dd = ts / seconds_in_day;
	civil_from_days(&year, &month, &day, dd);

	uint64_t weekday = weekday_from_days(dd);

	times->tm_sec = (int)second;
	times->tm_min = (int)minute;
	times->tm_hour = (int)hour;
	times->tm_mday= (int)day;
	times->tm_mon = (int)(month-1);    	// month starts with 0
	times->tm_year = (int)(year-1900); 	// years since 1900
	times->tm_wday = (int)weekday;
	times->tm_yday = 0;
	times->tm_isdst = 0;
} // end
