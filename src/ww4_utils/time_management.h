/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file time_management.h
 * @brief Routines for management of date and time.
 * @details This header defines the DateTime structure and TimeManagement class,
 *          providing utilities for calendar calculations, time increments,
 *          and conversions between various time formats including
 *          YYYYMMDD/HHMMSS, DateArray (8-integer array), and Julian Days.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-03-11
 * @date Last update : 2026-10-22
 * @note This file is converted from WAVEWATCH III (WW3) source file
 *       w3timemd.F90. Original author in WW3: Hendrik L. Tolman.
 */

#pragma once

#include <array>
#include <chrono>
#include <optional>
#include <string>
#include <string_view>

namespace ww4_utils {

// --- DateTime ---------------------------------------------------------------
/**
 * @struct DateTime
 * @brief Numerical representation of date and time.
 * @details Stores date as YYYYMMDD and time as HHMMSS.ssssss.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-03-11
 * @date Last update : 2026-09-28
 * @var DateTime::ymd
 * @brief Current date in YYYYMMDD format.
 * @var DateTime::hms
 * @brief Current time in HHMMSS.ssssss format.
 */
struct DateTime {
  int ymd;
  double hms;

  constexpr bool operator==(const DateTime &other) const noexcept {
    return ymd == other.ymd && hms == other.hms;
  }

  constexpr bool operator!=(const DateTime &other) const noexcept {
    return !(*this == other);
  }
};

// --- parseDateTimeString ----------------------------------------------------
/**
 * @brief Helper to parse a date-time string in "YYYYMMDD HHMMSS" format.
 * @param s The string view to parse.
 * @return A DateTime structure if successful, or std::nullopt.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-02
 * @date Last update : 2026-10-22
 */
std::optional<DateTime> parseDateTimeString(std::string_view s);

// --- DateArray --------------------------------------------------------------
using DateArray = std::array<int, 8>;

// --- TimeManagement ---------------------------------------------------------
/**
 * @class TimeManagement
 * @brief Routines for management of date and time, converted from WW3
 * w3timemd.F90.
 * @details Provides static methods for time arithmetic, calendar conversions,
 *          and high-precision profiling. Supports multiple calendar systems:
 *          Standard (Gregorian), NoLeap (365-day), and ThreeSixtyDay.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-03-11
 * @date Last update : 2026-09-28
 */
class TimeManagement {
public:
  // --- CalendarType ---------------------------------------------------------
  /**
   * @enum CalendarType
   * @brief Supported calendar systems.
   * @details Defines the different calendar rules used for date calculations.
   * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
   * @author Contributors: Jules (Agentic AI)
   * @date Initial, 2026-03-11
   * @date Last update : 2026-09-28
   * @var CalendarType::Standard
   * @brief Standard Gregorian calendar.
   * @var CalendarType::NoLeap
   * @brief 365-day calendar without leap years.
   * @var CalendarType::ThreeSixtyDay
   * @brief 360-day calendar with 12 months of 30 days each.
   */
  enum class CalendarType { Standard, NoLeap, ThreeSixtyDay };

  /**
   * @brief Sets the current calendar type.
   * @param type The calendar type to use for all calculations.
   */
  static void setCalendarType(const CalendarType type) noexcept;

  /**
   * @brief Gets the current calendar type.
   * @return The currently set calendar type.
   */
  static CalendarType getCalendarType() noexcept;

  /**
   * @brief Resets all persistent static members to their default values.
   */
  static void reset() noexcept;

  /**
   * @brief Increment a date and time with a given number of seconds.
   * @param[in,out] time Current date and time.
   * @param[in] dtime Time step in seconds.
   */
  static void incrementDateTime(DateTime &time, const double dtime) noexcept;

  /**
   * @brief Increment date in YYYYMMDD format by +/- 1 day.
   * @param ymd Old date in YYYYMMDD format.
   * @param adjustment +/- 1 (Day adjustment).
   * @return New date in YYYYMMDD format.
   */
  static int incrementDateByDay(const int ymd, const int adjustment) noexcept;

  /**
   * @brief Calculate difference in seconds between two DateTime structures.
   * @param time1 First date/time.
   * @param time2 Second date/time.
   * @return Difference (time2 - time1) in seconds.
   */
  static double differenceInSeconds(const DateTime &time1,
                                    const DateTime &time2) noexcept;

  /**
   * @brief Calculate difference in seconds between two DateArrays.
   * @param t1 First date/time array.
   * @param t2 Second date/time array.
   * @return Difference (t2 - t1) in seconds.
   */
  static double differenceInSeconds(const DateArray &t1,
                                    const DateArray &t2) noexcept;

  /**
   * @brief Captures present date and time into a DateArray.
   * @param[out] dateArray Date array to fill.
   */
  static void getSystemDateArray(DateArray &dateArray) noexcept;

  /**
   * @brief Calculates elapsed time since a reference date.
   * @param[in] referenceDate Reference date array.
   * @param[out] elapsedTime Elapsed time in seconds.
   */
  static void getElapsedTimeSince(const DateArray &referenceDate,
                                  double &elapsedTime) noexcept;

  /**
   * @brief Gets present date and time as a DateTime structure.
   * @return Current date and time.
   */
  static DateTime getPresentDateTime() noexcept;

  /**
   * @brief Convert date in YYYYMMDD format to ordinal day of the year.
   * @param ymd Date in YYYYMMDD format.
   * @return Day of year (1-366).
   */
  static int getDayOfYear(const int ymd) noexcept;

  /**
   * @brief Converts numerical time to a readable string.
   * @param time Date and time.
   * @return Readable string representation.
   */
  static std::string toFormattedString(const DateTime &time);

  /**
   * @brief Calculate Julian day from day, month, and year.
   * @param day Day of month.
   * @param month Month.
   * @param year Year.
   * @return Julian day number.
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
   * @brief Transform Julian day to day, month, and year.
   * @param[in] julian Julian day.
   * @param[out] day Day of month.
   * @param[out] month Month.
   * @param[out] year Year.
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
   * @brief Initialize profiling timer.
   */
  static void initializeProfiling() noexcept;

  /**
   * @brief Get profiling wall-clock time in seconds.
   * @return Elapsed time in seconds.
   */
  static double getProfilingTime() noexcept;

  /**
   * @brief Convert DateTime to DateArray.
   * @param[in] time Date and time.
   * @param[out] dateArray Date array.
   * @param[out] errorCode Error code (0 for success).
   */
  static void dateTimeToDateArray(const DateTime &time, DateArray &dateArray,
                                  int &errorCode) noexcept;

  /**
   * @brief Convert DateArray to DateTime.
   * @param[in] dateArray Date array.
   * @param[out] time Date and time.
   * @param[out] errorCode Error code (0 for success).
   */
  static void dateArrayToDateTime(const DateArray &dateArray, DateTime &time,
                                  int &errorCode) noexcept;

  /**
   * @brief Convert DateArray to Julian Day.
   * @param[in] dateArray Date array.
   * @param[out] julian Julian day.
   * @param[out] errorCode Error code (0 for success).
   */
  static void dateArrayToJulianDay(const DateArray &dateArray, double &julian,
                                   int &errorCode) noexcept;

  /**
   * @brief Convert Julian Day to DateArray.
   * @param[in] julian Julian day.
   * @param[out] dateArray Date array.
   * @param[out] errorCode Error code (0 for success).
   */
  static void julianDayToDateArray(const double julian, DateArray &dateArray,
                                   int &errorCode) noexcept;

  /**
   * @brief Calculate difference in days between two DateArrays.
   * @param t1 First date array.
   * @param t2 Second date array.
   * @return Difference (t2 - t1) in days.
   */
  static double differenceInDays(const DateArray &t1,
                                 const DateArray &t2) noexcept;

  /**
   * @brief Convert time units attribute string to DateArray.
   * @param units Units attribute string.
   * @param[out] dateArray Date array.
   * @param[out] errorCode Error code (0 for success).
   */
  static void parseUnitsToDateArray(const std::string_view units,
                                    DateArray &dateArray,
                                    int &errorCode) noexcept;

  /**
   * @brief Convert DateTime to ISO8601 string.
   * @param time Date and time.
   * @return ISO8601 string.
   */
  static std::string toIsoString(const DateTime &time);

  /**
   * @brief Gives date as hours since Julian Day 0.
   * @param time Date and time.
   * @return Hours since reference.
   */
  static double time2hours(const DateTime &time) noexcept;

private:
  static CalendarType m_calendarType;
  static DateArray m_profilingBase;
  static bool m_profilingInitialized;
  static std::chrono::steady_clock::time_point m_steadyBase;
};

} // namespace ww4_utils
