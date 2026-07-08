/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file time_management.cpp
 * @brief Implementation of time management routines.
 * @details This file implements the methods defined in the TimeManagement
 * class, providing robust handling of multiple calendar types and precise time
 * calculations.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Hendrik L. Tolman, Jules (Agentic AI)
 * @date Initial, 2026-03-11
 * @date Last update : 2026-07-07
 */

#include "ww4_utils/time_management.h"
#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

/**
 * @namespace ww4_utils
 * @brief Utilities for WAVEWATCH IV.
 * @details Contains core utility functions and classes for the WW4 project,
 *          focusing on high-performance and memory-safe implementations.
 */
namespace ww4_utils {

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
 * @date Last update : 2026-07-07
 * @fn TimeManagement::computeJulianDay
 * @brief Calculate the Julian day from a given date.
 * @details Computes the Julian Day Number for the Gregorian/Julian calendar.
 *          Converted from WW3 function JULDAY.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @param day Day of month.
 * @param month Month.
 * @param year Year.
 * @return Julian day number.
 * @pre year must not be zero.
 * @fn TimeManagement::computeCalendarDate
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

TimeManagement::CalendarType TimeManagement::m_calendarType =
    TimeManagement::CalendarType::Standard;
DateArray TimeManagement::m_profilingBase = {0, 0, 0, 0, 0, 0, 0, 0};
bool TimeManagement::m_profilingInitialized = false;
std::chrono::steady_clock::time_point TimeManagement::m_steadyBase;

/**
 * @brief Sets the current calendar type.
 * @details Updates the global calendar state used by all static methods.
 * @param type The calendar type to use for all calculations.
 * @pre type must be one of the CalendarType values.
 * @post The global calendar type is updated.
 */
void TimeManagement::setCalendarType(const CalendarType type) noexcept {
  m_calendarType = type;
}

/**
 * @brief Gets the current calendar type.
 * @details Returns the global calendar state.
 * @return The currently set calendar type.
 */
TimeManagement::CalendarType TimeManagement::getCalendarType() noexcept {
  return m_calendarType;
}

/**
 * @brief Resets all persistent static members to their default values.
 * @details Used during program finalization to ensure no stale data remains.
 * @post m_calendarType, m_profilingBase, m_profilingInitialized, and
 *       m_steadyBase are reset.
 */
void TimeManagement::reset() noexcept {
  m_calendarType = CalendarType::Standard;
  m_profilingBase = {0, 0, 0, 0, 0, 0, 0, 0};
  m_profilingInitialized = false;
  m_steadyBase = std::chrono::steady_clock::time_point();
}

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
void TimeManagement::incrementDateTime(DateTime &time,
                                       const double dtime) noexcept {
  const int nymd_init = time.ymd;
  int nymd = nymd_init;
  const double hms = time.hms;

  if (dtime == 0.0) {
    nymd = incrementDateByDay(nymd, -1);
    nymd = incrementDateByDay(nymd, 1);
  }

  const int hours = static_cast<int>(hms) / 10000;
  const int minutes = (static_cast<int>(hms) % 10000) / 100;
  const double seconds = hms - static_cast<double>(hours) * 10000.0 -
                         static_cast<double>(minutes) * 100.0;

  double total_seconds = static_cast<double>(hours) * 3600.0 +
                         static_cast<double>(minutes) * 60.0 + seconds + dtime;

  while (total_seconds >= 86400.0) {
    total_seconds -= 86400.0;
    nymd = incrementDateByDay(nymd, 1);
  }

  while (total_seconds < 0.0) {
    total_seconds += 86400.0;
    nymd = incrementDateByDay(nymd, -1);
  }

  const int f_hours = static_cast<int>(total_seconds) / 3600;
  const int f_minutes = (static_cast<int>(total_seconds) % 3600) / 60;
  const double f_seconds = std::fmod(total_seconds, 60.0);

  time.ymd = nymd;
  time.hms = static_cast<double>(f_hours) * 10000.0 +
             static_cast<double>(f_minutes) * 100.0 + f_seconds;
}

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
int TimeManagement::incrementDateByDay(const int ymd,
                                       const int adjustment) noexcept {
  const int ny_init = ymd / 10000;
  int ny = ny_init;
  const int nm_init = (ymd % 10000) / 100;
  int nm = std::clamp(nm_init, 1, 12);
  const int nd_init = (ymd % 100) + adjustment;
  int nd = nd_init;

  std::array<int, 12> ndpm{};
  if (m_calendarType == CalendarType::ThreeSixtyDay) {
    ndpm.fill(30);
  } else {
    static constexpr std::array<int, 12> standard_ndpm = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    ndpm = standard_ndpm;
  }

  const auto is_leap = [](const int year, const CalendarType type) noexcept {
    if (type != CalendarType::Standard)
      return false;
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
  };

  const bool leap = is_leap(ny, m_calendarType);

  if (nd == 0) {
    nm--;
    if (nm == 0) {
      nm = 12;
      ny--;
    }
    nd = ndpm[static_cast<size_t>(nm - 1)];
    if (nm == 2 && leap)
      nd = 29;
  }

  if (!(nd == 29 && nm == 2 && leap)) {
    if (nd > ndpm[static_cast<size_t>(nm - 1)]) {
      nd = 1;
      nm++;
      if (nm > 12) {
        nm = 1;
        ny++;
      }
    }
  }

  return ny * 10000 + nm * 100 + nd;
}

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
double TimeManagement::differenceInSeconds(const DateTime &time1,
                                           const DateTime &time2) noexcept {
  const int ny1 = time1.ymd / 10000;
  const int nd1 = getDayOfYear(time1.ymd);
  const int h1 = static_cast<int>(time1.hms) / 10000;
  const int m1 = (static_cast<int>(time1.hms) % 10000) / 100;
  const double s1 = std::fmod(time1.hms, 100.0);
  const double ns1 =
      static_cast<double>(h1) * 3600.0 + static_cast<double>(m1) * 60.0 + s1;

  const int ny2 = time2.ymd / 10000;
  const int nd2 = getDayOfYear(time2.ymd);
  const int h2 = static_cast<int>(time2.hms) / 10000;
  const int m2 = (static_cast<int>(time2.hms) % 10000) / 100;
  const double s2 = std::fmod(time2.hms, 100.0);
  const double ns2 =
      static_cast<double>(h2) * 3600.0 + static_cast<double>(m2) * 60.0 + s2;

  const int nd_init = nd2 - nd1;
  int nd = nd_init;

  if (ny1 != ny2) {
    const int nst = (ny2 > ny1) ? 1 : -1;
    int current_ny = ny1;
    while (current_ny != ny2) {
      if (nst > 0) {
        if (m_calendarType == CalendarType::ThreeSixtyDay) {
          nd += 360;
        } else if (m_calendarType == CalendarType::NoLeap) {
          nd += 365;
        } else {
          const bool leap = (current_ny % 400 == 0) ||
                            (current_ny % 4 == 0 && current_ny % 100 != 0);
          nd += leap ? 366 : 365;
        }
        current_ny++;
      } else {
        current_ny--;
        if (m_calendarType == CalendarType::ThreeSixtyDay) {
          nd -= 360;
        } else if (m_calendarType == CalendarType::NoLeap) {
          nd -= 365;
        } else {
          const bool leap = (current_ny % 400 == 0) ||
                            (current_ny % 4 == 0 && current_ny % 100 != 0);
          nd -= leap ? 366 : 365;
        }
      }
    }
  }

  const double ns = ns2 - ns1;
  return ns + 86400.0 * static_cast<double>(nd);
}

/**
 * @brief Convert date in YYYYMMDD format to Julian day within the year.
 * @details Returns the ordinal day of the year (1-365 or 1-366).
 *          Converted from WW3 function MYMD21.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @param ymd Date in YYYYMMDD format.
 * @return Julian day (1-366).
 */
int TimeManagement::getDayOfYear(const int ymd) noexcept {
  const int ny = ymd / 10000;
  const int nm_init = (ymd % 10000) / 100;
  int nm = nm_init;
  const int nd_init = ymd % 100;
  int nd = nd_init;

  static constexpr std::array<int, 12> standard_ndpm = {31, 28, 31, 30, 31, 30,
                                                        31, 31, 30, 31, 30, 31};

  const bool leap = (m_calendarType == CalendarType::Standard)
                        ? ((ny % 400 == 0) || (ny % 4 == 0 && ny % 100 != 0))
                        : false;

  if (nm > 2 && leap)
    nd++;

  while (nm > 1) {
    nm--;
    if (m_calendarType == CalendarType::ThreeSixtyDay) {
      nd += 30;
    } else {
      nd += standard_ndpm[static_cast<size_t>(nm - 1)];
    }
  }

  return nd;
}

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
void TimeManagement::dateTimeToDateArray(const DateTime &time,
                                         DateArray &dateArray,
                                         int &errorCode) noexcept {
  dateArray[0] = time.ymd / 10000;
  dateArray[1] = (time.ymd / 100) % 100;
  dateArray[2] = time.ymd % 100;
  dateArray[3] = 0; // UTC
  dateArray[4] = static_cast<int>(time.hms) / 10000;
  dateArray[5] = (static_cast<int>(time.hms) / 100) % 100;
  dateArray[6] = static_cast<int>(time.hms) % 100;
  dateArray[7] =
      static_cast<int>(std::round(std::fmod(time.hms, 1.0) * 1000.0));
  errorCode = 0;
}

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
void TimeManagement::dateArrayToDateTime(const DateArray &dateArray,
                                         DateTime &time,
                                         int &errorCode) noexcept {
  const int ymd_val = dateArray[0] * 10000 + dateArray[1] * 100 + dateArray[2];
  const double hms_val = static_cast<double>(dateArray[4]) * 10000.0 +
                         static_cast<double>(dateArray[5]) * 100.0 +
                         static_cast<double>(dateArray[6]) +
                         dateArray[7] / 1000.0;
  time.ymd = ymd_val;
  time.hms = hms_val;
  errorCode = 0;
}

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
void TimeManagement::dateArrayToJulianDay(const DateArray &dateArray,
                                          double &julian,
                                          int &errorCode) noexcept {
  const int year = dateArray[0];
  const int month = dateArray[1];
  const int day = dateArray[2];
  const int hour = dateArray[4];
  const int minute = dateArray[5];
  const double second =
      dateArray[6] - dateArray[3] * 60.0 + dateArray[7] / 1000.0;

  if (m_calendarType == CalendarType::ThreeSixtyDay) {
    julian = (year - 1800) * 360.0 + (month - 1) * 30.0 + (day - 1) +
             hour / 24.0 + minute / 1440.0 + second / 86400.0;
    errorCode = 0;
    return;
  }

  if (year == 0 || year < -4713) {
    errorCode = -1;
    return;
  }

  const int a = (14 - month) / 12;
  const int y = year + 4800 - a;
  const int m = month + 12 * a - 3;
  const int jdn =
      day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
  julian = static_cast<double>(jdn) + (hour - 12) / 24.0 + minute / 1440.0 +
           second / 86400.0;
  errorCode = (julian < 0.0) ? 1 : 0;
}

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
void TimeManagement::julianDayToDateArray(const double julian,
                                          DateArray &dateArray,
                                          int &errorCode) noexcept {
  if (m_calendarType == CalendarType::Standard && julian < 0.0) {
    errorCode = 1;
    return;
  }

  const int ijul_init = static_cast<int>(std::floor(julian));
  int ijul = ijul_init;
  const double second_init = (julian - static_cast<double>(ijul)) * 86400.0;
  double second = second_init;
  const int tz = 0; // UTC

  if (m_calendarType == CalendarType::Standard) {
    if (second >= 43200.0) {
      ijul++;
      second -= 43200.0;
    } else {
      second += 43200.0;
    }
  }

  if (second >= 86400.0) {
    ijul++;
    second -= 86400.0;
  }

  const int minute_init = static_cast<int>(second / 60.0);
  const double sec = std::fmod(second, 60.0);
  const int hour = minute_init / 60;
  const int minute = minute_init % 60;

  int year_val, month_val, day_val;
  if (m_calendarType == CalendarType::ThreeSixtyDay) {
    year_val = static_cast<int>(julian / 360.0) + 1800;
    month_val = (static_cast<int>(julian / 30.0) % 12) + 1;
    day_val = (static_cast<int>(julian) % 30) + 1;
  } else {
    const int jalpha = static_cast<int>(
        (static_cast<double>(ijul - 1867216) - 0.25) / 36524.25);
    const int ja = ijul + 1 + jalpha - static_cast<int>(0.25 * jalpha);
    const int jb = ja + 1524;
    const int jc = static_cast<int>(
        6680.0 + (static_cast<double>(jb - 2439870) - 122.1) / 365.25);
    const int jd = 365 * jc + static_cast<int>(0.25 * jc);
    const int je = static_cast<int>(static_cast<double>(jb - jd) / 30.6001);
    day_val = jb - jd - static_cast<int>(30.6001 * je);
    month_val = je - 1;
    if (month_val > 12)
      month_val -= 12;
    year_val = jc - 4715;
    if (month_val > 2)
      year_val--;
    if (year_val <= 0)
      year_val--;
  }

  dateArray[0] = year_val;
  dateArray[1] = month_val;
  dateArray[2] = day_val;
  dateArray[3] = tz;
  dateArray[4] = hour;
  dateArray[5] = minute;
  dateArray[6] = static_cast<int>(sec);
  dateArray[7] = static_cast<int>(std::round(std::fmod(sec, 1.0) * 1000.0));
  errorCode = 0;
}

/**
 * @brief Calculate the difference in seconds between two date/time arrays.
 * @details Computes t2 - t1 in seconds for arrays in DATE_AND_TIME format.
 *          Converted from WW3 function TDIFF.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @param t1 First date/time array (DATE_AND_TIME format).
 * @param t2 Second date/time array (DATE_AND_TIME format).
 * @return Difference (t2 - t1) in seconds.
 */
double TimeManagement::differenceInSeconds(const DateArray &t1,
                                           const DateArray &t2) noexcept {
  const double diff_s = 86400.0 * differenceInDays(t1, t2);
  return diff_s;
}

/**
 * @brief Captures the present date and time.
 * @details Fills a DateArray with the current UTC system time.
 *          Matches Fortran's DATE_AND_TIME behavior.
 *          Converted from WW3 routine W3DATE.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @param[out] dateArray Date array to be filled.
 * @post dateArray contains the current UTC system time.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-03-11
 * @date Last update : 2026-07-07
 */
void TimeManagement::getSystemDateArray(DateArray &dateArray) noexcept {
  const auto now = std::chrono::system_clock::now();
  const auto tt = std::chrono::system_clock::to_time_t(now);

  std::tm gmt{};
#ifdef _WIN32
  gmtime_s(&gmt, &tt);
#else
  gmtime_r(&tt, &gmt);
#endif

  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

  dateArray[0] = gmt.tm_year + 1900;
  dateArray[1] = gmt.tm_mon + 1;
  dateArray[2] = gmt.tm_mday;
  dateArray[3] = 0; // UTC
  dateArray[4] = gmt.tm_hour;
  dateArray[5] = gmt.tm_min;
  dateArray[6] = gmt.tm_sec;
  dateArray[7] = static_cast<int>(ms.count());
}

/**
 * @brief Calculates the elapsed time since a reference date.
 * @details Computes the difference in seconds between the current system
 *          time and the provided reference date.
 *          Converted from WW3 routine W3DTIM.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @param[in] referenceDate Reference date array (DATE_AND_TIME format).
 * @param[out] elapsedTime Elapsed time in seconds.
 * @post elapsedTime contains the difference in seconds.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-03-11
 * @date Last update : 2026-07-07
 */
void TimeManagement::getElapsedTimeSince(const DateArray &referenceDate,
                                         double &elapsedTime) noexcept {
  DateArray now_dat{};
  getSystemDateArray(now_dat);
  const double elapsed_val = differenceInSeconds(referenceDate, now_dat);
  elapsedTime = elapsed_val;
}

/**
 * @brief Gets the present date and time as a DateTime structure.
 * @details Retrieves the current UTC system time.
 * @return Current date and time in DateTime format.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-03-11
 * @date Last update : 2026-07-07
 */
DateTime TimeManagement::getPresentDateTime() noexcept {
  DateArray dat_arr{};
  getSystemDateArray(dat_arr);
  DateTime dt_val{};
  int err_code;
  dateArrayToDateTime(dat_arr, dt_val, err_code);
  return dt_val;
}

/**
 * @brief Calculate the difference in days between two DateArrays.
 * @details Computes t2 - t1 in days, supporting different calendars.
 *          Converted from WW3 function TSUB.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @param t1 First date array.
 * @param t2 Second date array.
 * @return Difference (t2 - t1) in days.
 */
double TimeManagement::differenceInDays(const DateArray &t1,
                                        const DateArray &t2) noexcept {
  if (m_calendarType == CalendarType::ThreeSixtyDay) {
    const int ad =
        (t2[0] - t1[0]) * 360 + (t2[1] - t1[1]) * 30 + (t2[2] - t1[2]);
    const double e1 =
        3600.0 * t1[4] + 60.0 * (t1[5] - t1[3]) + t1[6] + t1[7] / 1000.0;
    const double e2 =
        3600.0 * t2[4] + 60.0 * (t2[5] - t2[3]) + t2[6] + t2[7] / 1000.0;
    return static_cast<double>(ad) + (e2 - e1) / 86400.0;
  } else {
    const int a1 = (14 - t1[1]) / 12;
    const int b1 = t1[0] + 4800 - a1;
    const int c1 = t1[1] + 12 * a1 - 3;
    const int d1_init = t1[2] + (153 * c1 + 2) / 5 + 365 * b1;
    const int d1 = (m_calendarType == CalendarType::Standard)
                       ? (d1_init + b1 / 4 - b1 / 100 + b1 / 400)
                       : d1_init;
    const double e1 =
        3600.0 * t1[4] + 60.0 * (t1[5] - t1[3]) + t1[6] + t1[7] / 1000.0;

    const int a2 = (14 - t2[1]) / 12;
    const int b2 = t2[0] + 4800 - a2;
    const int c2 = t2[1] + 12 * a2 - 3;
    const int d2_init = t2[2] + (153 * c2 + 2) / 5 + 365 * b2;
    const int d2 = (m_calendarType == CalendarType::Standard)
                       ? (d2_init + b2 / 4 - b2 / 100 + b2 / 400)
                       : d2_init;
    const double e2 =
        3600.0 * t2[4] + 60.0 * (t2[5] - t2[3]) + t2[6] + t2[7] / 1000.0;

    return static_cast<double>(d2 - d1) + (e2 - e1) / 86400.0;
  }
}

/**
 * @brief Initialize profiling.
 * @details Captures the current steady clock time for high-precision
 * measurement. Converted from WW3 routine PRINIT.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @post m_profilingInitialized is true.
 */
void TimeManagement::initializeProfiling() noexcept {
  m_steadyBase = std::chrono::steady_clock::now();
  m_profilingInitialized = true;
}

/**
 * @brief Get profiling wall-clock time in seconds.
 * @details Calculates the elapsed time since initializeProfiling() was
 * called. Converted from WW3 function PRTIME.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @return Time since initializeProfiling() in seconds, or -1.0 if not
 * initialized.
 */
double TimeManagement::getProfilingTime() noexcept {
  if (!m_profilingInitialized)
    return -1.0;
  const auto now_prof = std::chrono::steady_clock::now();
  const std::chrono::duration<double> diff_prof = now_prof - m_steadyBase;
  return diff_prof.count();
}

/**
 * @brief Converts numerical time to a readable string.
 * @details Formats the DateTime as "YYYY/MM/DD HH:MM:SS UTC".
 *          Converted from WW3 function STME21.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @param time Date and time.
 * @return Readable string.
 */
std::string TimeManagement::toFormattedString(const DateTime &time) {
  if (time.ymd < 0) {
    return " date and time not set.";
  }
  const int iy = time.ymd / 10000;
  const int imo = (time.ymd / 100) % 100;
  const int id = time.ymd % 100;
  const int ih = static_cast<int>(time.hms) / 10000;
  const int imi = (static_cast<int>(time.hms) / 100) % 100;
  const int is = static_cast<int>(std::fmod(time.hms, 100.0));

  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(4) << iy << "/" << std::setw(2) << imo
      << "/" << std::setw(2) << id << " " << std::setw(2) << ih << ":"
      << std::setw(2) << imi << ":" << std::setw(2) << is << " UTC";
  return oss.str();
}

/**
 * @brief Convert DateTime to ISO8601 time string.
 * @details Formats as "YYYY-MM-DDTHH:MM:SS".
 *          Converted from WW3 function T2ISO.
 * @note Original author in WW3: Hendrik L. Tolman.
 * @param time Date and time.
 * @return ISO8601 string.
 */
std::string TimeManagement::toIsoString(const DateTime &time) {
  const int iy = time.ymd / 10000;
  const int imo = (time.ymd / 100) % 100;
  const int id = time.ymd % 100;
  const int ih = static_cast<int>(time.hms) / 10000;
  const int imi = (static_cast<int>(time.hms) / 100) % 100;
  const int is = static_cast<int>(std::fmod(time.hms, 100.0));

  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(4) << iy << "-" << std::setw(2) << imo
      << "-" << std::setw(2) << id << "T" << std::setw(2) << ih << ":"
      << std::setw(2) << imi << ":" << std::setw(2) << is;
  return oss.str();
}

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
void TimeManagement::parseUnitsToDateArray(const std::string_view units,
                                           DateArray &dateArray,
                                           int &errorCode) noexcept {
  dateArray.fill(0);
  errorCode = 1;

  const size_t since_pos = units.find("since ");
  if (since_pos == std::string_view::npos)
    return;

  const std::string_view date_part = units.substr(since_pos + 6);

  const auto parse_int = [](const std::string_view s,
                            int &val) -> std::string_view {
    if (s.empty())
      return s;
    const char *first = s.data();
    const char *last = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(first, last, val);
    if (ec != std::errc())
      return s;
    const size_t consumed = static_cast<size_t>(ptr - first);
    if (consumed < s.size() && (s[consumed] == '-' || s[consumed] == ' ' ||
                                s[consumed] == ':' || s[consumed] == 'T')) {
      return s.substr(consumed + 1);
    }
    return s.substr(consumed);
  };

  int y_val = 0, m_val = 0, d_val = 0;
  std::string_view remaining = date_part;
  remaining = parse_int(remaining, y_val);
  remaining = parse_int(remaining, m_val);
  remaining = parse_int(remaining, d_val);

  if (y_val != 0 && m_val != 0 && d_val != 0) {
    dateArray[0] = y_val;
    dateArray[1] = m_val;
    dateArray[2] = d_val;
    errorCode = 0;

    int h_val = 0, mi_val = 0, s_val = 0;
    remaining = parse_int(remaining, h_val);
    remaining = parse_int(remaining, mi_val);
    remaining = parse_int(remaining, s_val);

    dateArray[4] = h_val;
    dateArray[5] = mi_val;
    dateArray[6] = s_val;
  }
}

/**
 * @brief Gives date as a real number (hours since reference).
 * @details Computes the number of hours since Julian Day 0.
 * @param time Date and time.
 * @return Hours since Julian Day 0.
 */
double TimeManagement::time2hours(const DateTime &time) noexcept {
  const int iy = time.ymd / 10000;
  const int imo = (time.ymd / 100) % 100;
  const int id = time.ymd % 100;
  const int ih = static_cast<int>(time.hms) / 10000;
  const int imi = (static_cast<int>(time.hms) / 100) % 100;
  const double is = std::fmod(time.hms, 100.0);
  const int jday = computeJulianDay(id, imo, iy);
  return 24.0 * jday + ih + (imi * 60.0 + is) / 3600.0;
}

} // namespace ww4_utils
