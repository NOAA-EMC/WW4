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
 * YYYYMMDD/HHMMSS, DateArray (8-integer array), and Julian Days.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-03-11
 * @date Last update : 2026-07-07
 *
 * @note This file is converted from WAVEWATCH III (WW3) source file
 *       w3timemd.F90. Original author in WW3: Hendrik L. Tolman.
 */

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <string_view>

namespace ww4_utils {

/**
 * @struct DateTime
 * @brief Numerical representation of date and time.
 * @details Stores date as YYYYMMDD and time as HHMMSS.ssssss.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
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

using DateArray = std::array<int, 8>;

class TimeManagement {
public:
  /**
   * @enum CalendarType
   * @brief Supported calendar systems.
   * @details Defines the different calendar rules used for date calculations.
   * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
   * @author Contributors: Jules (Agentic AI)
   * @var CalendarType::Standard
   * @brief Standard Gregorian calendar.
   * @var CalendarType::NoLeap
   * @brief 365-day calendar without leap years.
   * @var CalendarType::ThreeSixtyDay
   * @brief 360-day calendar with 12 months of 30 days each.
   */
  enum class CalendarType { Standard, NoLeap, ThreeSixtyDay };

  static void setCalendarType(const CalendarType type) noexcept;

  static CalendarType getCalendarType() noexcept;

  static void reset() noexcept;

  static void incrementDateTime(DateTime &time, const double dtime) noexcept;

  static int incrementDateByDay(const int ymd, const int adjustment) noexcept;

  static double differenceInSeconds(const DateTime &time1,
                                    const DateTime &time2) noexcept;

  static double differenceInSeconds(const DateArray &t1,
                                    const DateArray &t2) noexcept;

  static void getSystemDateArray(DateArray &dateArray) noexcept;

  static void getElapsedTimeSince(const DateArray &referenceDate,
                                  double &elapsedTime) noexcept;

  static DateTime getPresentDateTime() noexcept;

  static int getDayOfYear(const int ymd) noexcept;

  static std::string toFormattedString(const DateTime &time);

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

  static void initializeProfiling() noexcept;

  static double getProfilingTime() noexcept;

  static void dateTimeToDateArray(const DateTime &time, DateArray &dateArray,
                                  int &errorCode) noexcept;

  static void dateArrayToDateTime(const DateArray &dateArray, DateTime &time,
                                  int &errorCode) noexcept;

  static void dateArrayToJulianDay(const DateArray &dateArray, double &julian,
                                   int &errorCode) noexcept;

  static void julianDayToDateArray(const double julian, DateArray &dateArray,
                                   int &errorCode) noexcept;

  static double differenceInDays(const DateArray &t1,
                                 const DateArray &t2) noexcept;

  static void parseUnitsToDateArray(const std::string_view units,
                                    DateArray &dateArray,
                                    int &errorCode) noexcept;

  static std::string toIsoString(const DateTime &time);

  static double time2hours(const DateTime &time) noexcept;

private:
  static CalendarType m_calendarType;
  static DateArray m_profilingBase;
  static bool m_profilingInitialized;
  static std::chrono::steady_clock::time_point m_steadyBase;
};

} // namespace ww4_utils
