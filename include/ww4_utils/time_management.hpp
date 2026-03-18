/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file time_management.hpp
 * @brief Routines for management of date and time.
 * @details This header defines the DateTime structure and TimeManagement class,
 *          providing utilities for calendar calculations, time increments,
 *          and conversions between various time formats including
 * YYYYMMDD/HHMMSS, DateArray (8-integer array), and Julian Days.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * @author Hendrik L. Tolman (Initial, 2026-03-11)
 * @author Aldgisl, Hendrik L. Tolman (Last Update, 2026-03-18)
 * @date 2026-03-18
 */

#ifndef WW4_UTILS_TIME_MANAGEMENT_HPP
#define WW4_UTILS_TIME_MANAGEMENT_HPP

#include <array>
#include <chrono>
#include <span>
#include <string>
#include <string_view>

/**
 * @namespace ww4_utils
 * @brief Utilities for WAVEWATCH IV.
 * @details Contains core utility functions and classes for the WW4 project,
 *          focusing on high-performance and memory-safe implementations.
 */
namespace ww4_utils {

/**
 * @struct DateTime
 * @brief Numerical representation of date and time.
 * @details Stores date as YYYYMMDD and time as HHMMSS.ssssss.
 * @author Hendrik L. Tolman (Initial, 2026-03-11)
 * @author Aldgisl, Hendrik L. Tolman (Last Update, 2026-03-18)
 */
struct DateTime {
  int ymd;    ///< Current date in YYYYMMDD format.
  double hms; ///< Current time in HHMMSS.ssssss format.
};

/**
 * @brief Array of 8 integers matching Fortran DATE_AND_TIME format.
 * @details Index mapping:
 *          0: Year, 1: Month (1-12), 2: Day (1-31),
 *          3: Time difference with UTC in minutes,
 *          4: Hour (0-23), 5: Minute (0-59), 6: Seconds (0-60),
 *          7: Milliseconds (0-999).
 */
using DateArray = std::array<int, 8>;

/**
 * @class TimeManagement
 * @brief Routines for management of date and time, converted from WW3
 * w3timemd.F90.
 * @details Provides static methods for time arithmetic, calendar conversions,
 *          and high-precision profiling. Supports multiple calendar systems:
 *          Standard (Gregorian), NoLeap (365-day), and ThreeSixtyDay.
 * @author Hendrik L. Tolman (Initial, 2026-03-11)
 * @author Aldgisl, Hendrik L. Tolman (Last Update, 2026-03-18)
 */
class TimeManagement {
public:
  /**
   * @enum CalendarType
   * @brief Supported calendar systems.
   * @details Defines the different calendar rules used for date calculations.
   */
  enum class CalendarType {
    Standard,     ///< Gregorian calendar with leap years.
    NoLeap,       ///< 365-day calendar without leap years.
    ThreeSixtyDay ///< 360-day calendar with 30 days per month.
  };

  /**
   * @brief Sets the current calendar type.
   * @details Updates the global calendar state used by all static methods.
   * @param type The calendar type to use for all calculations.
   * @pre type must be one of the CalendarType values.
   * @post The global calendar type is updated.
   */
  static void setCalendarType(const CalendarType type) noexcept;

  /**
   * @brief Gets the current calendar type.
   * @details Returns the global calendar state.
   * @return The currently set calendar type.
   */
  static CalendarType getCalendarType() noexcept;

  /**
   * @brief Increment a date and time with a given number of seconds.
   * @details Updates the DateTime structure by adding a specified number of
   * seconds, handling day and month rollovers according to the active calendar.
   *          Converted from WW3 routine TICK21.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param[in,out] time Current date and time.
   * @param[in] dtime Time step in seconds.
   * @post time is updated by dtime.
   */
  static void incrementDateTime(DateTime &time, const double dtime) noexcept;

  /**
   * @brief Increment date in YYYYMMDD format by +/- 1 day.
   * @details Adjusts the date by one day forward or backward.
   *          Converted from WW3 function IYMD21.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param ymd Old date in YYYYMMDD format.
   * @param adjustment +/- 1 (Day adjustment).
   * @return New date in YYYYMMDD format.
   * @pre adjustment should be 1 or -1 for expected behavior.
   */
  static int incrementDateByDay(const int ymd, const int adjustment) noexcept;

  /**
   * @brief Calculate the difference in seconds between two date/time
   * structures.
   * @details Computes time2 - time1 in seconds, accounting for calendar
   * differences. Converted from WW3 function DSEC21.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param time1 First date/time.
   * @param time2 Second date/time.
   * @return Difference (time2 - time1) in seconds.
   */
  static double differenceInSeconds(const DateTime &time1,
                                    const DateTime &time2) noexcept;

  /**
   * @brief Calculate the difference in seconds between two date/time arrays.
   * @details Computes t2 - t1 in seconds for arrays in DATE_AND_TIME format.
   *          Converted from WW3 function TDIFF.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param t1 First date/time array (DATE_AND_TIME format).
   * @param t2 Second date/time array (DATE_AND_TIME format).
   * @return Difference (t2 - t1) in seconds.
   */
  static double differenceInSeconds(const std::span<const int, 8> t1,
                                    const std::span<const int, 8> t2) noexcept;

  /**
   * @brief Captures the present date and time.
   * @details Fills a DateArray with the current UTC system time.
   *          Matches Fortran's DATE_AND_TIME behavior.
   *          Converted from WW3 routine W3DATE.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param[out] dateArray Date array to be filled.
   * @post dateArray contains the current UTC system time.
   * @author Aldgisl, Hendrik L. Tolman (Initial, 2026-03-11)
   * @author Aldgisl, Hendrik L. Tolman (Last Update, 2026-03-18)
   * @date 2026-03-18
   */
  static void getSystemDateArray(const std::span<int, 8> dateArray) noexcept;

  /**
   * @brief Calculates the elapsed time since a reference date.
   * @details Computes the difference in seconds between the current system
   *          time and the provided reference date.
   *          Converted from WW3 routine W3DTIM.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param[in] referenceDate Reference date array (DATE_AND_TIME format).
   * @param[out] elapsedTime Elapsed time in seconds.
   * @post elapsedTime contains the difference in seconds.
   * @author Aldgisl, Hendrik L. Tolman (Initial, 2026-03-11)
   * @author Aldgisl, Hendrik L. Tolman (Last Update, 2026-03-18)
   * @date 2026-03-18
   */
  static void getElapsedTimeSince(const std::span<const int, 8> referenceDate,
                                  double &elapsedTime) noexcept;

  /**
   * @brief Gets the present date and time as a DateTime structure.
   * @details Retrieves the current UTC system time.
   * @return Current date and time in DateTime format.
   * @author Aldgisl, Hendrik L. Tolman (Initial, 2026-03-11)
   * @author Aldgisl, Hendrik L. Tolman (Last Update, 2026-03-18)
   * @date 2026-03-18
   */
  static DateTime getPresentDateTime() noexcept;

  /**
   * @brief Convert date in YYYYMMDD format to Julian day within the year.
   * @details Returns the ordinal day of the year (1-365 or 1-366).
   *          Converted from WW3 function MYMD21.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param ymd Date in YYYYMMDD format.
   * @return Julian day (1-366).
   */
  static int getDayOfYear(const int ymd) noexcept;

  /**
   * @brief Converts numerical time to a readable string.
   * @details Formats the DateTime as "YYYY/MM/DD HH:MM:SS UTC".
   *          Converted from WW3 function STME21.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param time Date and time.
   * @return Readable string.
   */
  static std::string toFormattedString(const DateTime &time);

  /**
   * @brief Calculate the Julian day from a given date.
   * @details Computes the Julian Day Number for the Gregorian/Julian calendar.
   *          Converted from WW3 function JULDAY.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param day Day of month.
   * @param month Month.
   * @param year Year.
   * @return Julian day number.
   * @pre year must not be zero.
   */
  static constexpr int computeJulianDay(const int day, const int month,
                                        const int year) noexcept {
    int jy = year;
    if (jy == 0)
      return -1; // No year zero
    if (jy < 0)
      jy++;
    int jm, jdn;
    if (month > 2) {
      jm = month + 1;
    } else {
      jy--;
      jm = month + 13;
    }
    jdn = static_cast<int>(365.25 * jy) + static_cast<int>(30.6001 * jm) + day +
          1720995;
    if (day + 31 * (month + 12 * year) >= (15 + 31 * (10 + 12 * 1582))) {
      const int ja = static_cast<int>(0.01 * jy);
      jdn = jdn + 2 - ja + static_cast<int>(0.25 * ja);
    }
    return jdn;
  }

  /**
   * @brief Transform Julian day to date.
   * @details Reverses the Julian Day calculation to retrieve day, month, and
   * year. Converted from WW3 routine CALDAT.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param[in] julian Julian day.
   * @param[out] day Day of month.
   * @param[out] month Month.
   * @param[out] year Year.
   * @post day, month, and year are populated with the calculated date.
   */
  static constexpr void computeCalendarDate(const int julian, int &day,
                                            int &month, int &year) noexcept {
    int ja;
    if (julian >= 2299161) {
      const int jalpha = static_cast<int>(
          (static_cast<double>(julian - 1867216) - 0.25) / 36524.25);
      ja = julian + 1 + jalpha - static_cast<int>(0.25 * jalpha);
    } else {
      ja = julian;
    }
    const int jb = ja + 1524;
    const int jc = static_cast<int>(
        6680.0 + (static_cast<double>(jb - 2439870) - 122.1) / 365.25);
    const int jd = 365 * jc + static_cast<int>(0.25 * jc);
    const int je = static_cast<int>(static_cast<double>(jb - jd) / 30.6001);
    day = jb - jd - static_cast<int>(30.6001 * je);
    month = (je < 14) ? (je - 1) : (je - 13);
    year = jc - 4715;
    if (month > 2)
      year--;
    if (year <= 0)
      year--;
  }

  /**
   * @brief Initialize profiling.
   * @details Captures the current steady clock time for high-precision
   * measurement. Converted from WW3 routine PRINIT.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @post m_profilingInitialized is true.
   */
  static void initializeProfiling() noexcept;

  /**
   * @brief Get profiling wall-clock time in seconds.
   * @details Calculates the elapsed time since initializeProfiling() was
   * called. Converted from WW3 function PRTIME.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @return Time since initializeProfiling() in seconds, or -1.0 if not
   * initialized.
   */
  static double getProfilingTime() noexcept;

  /**
   * @brief Convert DateTime to DateArray.
   * @details Decomposes DateTime into an 8-integer array.
   *          Converted from WW3 routine T2D.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param[in] time Date and time.
   * @param[out] dateArray Date array.
   * @param[out] errorCode Error code (0 for success).
   * @post dateArray is populated, errorCode is set.
   */
  static void dateTimeToDateArray(const DateTime &time,
                                  const std::span<int, 8> dateArray,
                                  int &errorCode) noexcept;

  /**
   * @brief Convert DateArray to DateTime.
   * @details Reconstructs DateTime from an 8-integer array.
   *          Converted from WW3 routine D2T.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param[in] dateArray Date array.
   * @param[out] time Date and time.
   * @param[out] errorCode Error code (0 for success).
   * @post time is populated, errorCode is set.
   */
  static void dateArrayToDateTime(const std::span<const int, 8> dateArray,
                                  DateTime &time, int &errorCode) noexcept;

  /**
   * @brief Convert DateArray to Julian Day.
   * @details Computes Julian Day from an 8-integer array.
   *          Converted from WW3 routine D2J.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param[in] dateArray Date array.
   * @param[out] julian Julian day.
   * @param[out] errorCode Error code (0 for success, -1/1 for errors).
   * @post julian is populated, errorCode is set.
   */
  static void dateArrayToJulianDay(const std::span<const int, 8> dateArray,
                                   double &julian, int &errorCode) noexcept;

  /**
   * @brief Convert Julian Day to DateArray.
   * @details Decomposes Julian Day into an 8-integer array.
   *          Converted from WW3 routine J2D.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param[in] julian Julian day.
   * @param[out] dateArray Date array.
   * @param[out] errorCode Error code (0 for success).
   * @post dateArray is populated, errorCode is set.
   */
  static void julianDayToDateArray(const double julian,
                                   const std::span<int, 8> dateArray,
                                   int &errorCode) noexcept;

  /**
   * @brief Calculate the difference in days between two DateArrays.
   * @details Computes t2 - t1 in days, supporting different calendars.
   *          Converted from WW3 function TSUB.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param t1 First date array.
   * @param t2 Second date array.
   * @return Difference (t2 - t1) in days.
   */
  static double differenceInDays(const std::span<const int, 8> t1,
                                 const std::span<const int, 8> t2) noexcept;

  /**
   * @brief Convert time units attribute to DateArray.
   * @details Parses a string like "seconds since 1970-01-01 00:00:00" into a
   * DateArray. Converted from WW3 routine U2D.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param units Units attribute.
   * @param[out] dateArray Date array.
   * @param[out] errorCode Error code (0 for success, 1 for error).
   * @post dateArray is populated if successful, errorCode is set.
   */
  static void parseUnitsToDateArray(const std::string_view units,
                                    const std::span<int, 8> dateArray,
                                    int &errorCode) noexcept;

  /**
   * @brief Convert DateTime to ISO8601 time string.
   * @details Formats as "YYYY-MM-DDTHH:MM:SS".
   *          Converted from WW3 function T2ISO.
   * @note Original author in WW3: Hendrik L. Tolman.
   * @param time Date and time.
   * @return ISO8601 string.
   */
  static std::string toIsoString(const DateTime &time);

  /**
   * @brief Gives date as a real number (hours since reference).
   * @details Computes the number of hours since Julian Day 0.
   * @param time Date and time.
   * @return Hours since Julian Day 0.
   */
  static double time2hours(const DateTime &time) noexcept;

private:
  static CalendarType m_calendarType; ///< Currently active calendar system.
  static DateArray m_profilingBase;   ///< Base time for profiling.
  static bool m_profilingInitialized; ///< Flag for profiling initialization.
  static std::chrono::steady_clock::time_point
      m_steadyBase; ///< Steady base for high precision.
};

} // namespace ww4_utils

#endif // WW4_UTILS_TIME_MANAGEMENT_HPP
