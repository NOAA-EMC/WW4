/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file L1_test_time_management.cpp
 * @brief Unit tests for time management routines using Google Test.
 * @details This file provides comprehensive unit testing for the TimeManagement
 * class, covering leap year cycles, fractional seconds, different calendars,
 *          and conversions.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-02-27
 * @date Last update : 2026-07-13
 */

#include "ww4_utils/time_management.h"
#include <chrono>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <vector>

using namespace ww4_utils;

class TimeManagementTest : public ::testing::Test {
protected:
  void SetUp() override {
    TimeManagement::setCalendarType(TimeManagement::CalendarType::Standard);
  }
};

TEST_F(TimeManagementTest, LeapYearCycles) {
  // 4-year cycle
  EXPECT_EQ(TimeManagement::incrementDateByDay(20040228, 1), 20040229);
  EXPECT_EQ(TimeManagement::incrementDateByDay(20030228, 1), 20030301);

  // 100-year cycle (not a leap year)
  EXPECT_EQ(TimeManagement::incrementDateByDay(19000228, 1), 19000301);

  // 400-year cycle (is a leap year)
  EXPECT_EQ(TimeManagement::incrementDateByDay(20000228, 1), 20000229);
  EXPECT_EQ(TimeManagement::incrementDateByDay(20000229, 1), 20000301);

  // incrementDateTime leap year check
  DateTime dt = {20000228, 235959.0};
  TimeManagement::incrementDateTime(dt, 1.0);
  EXPECT_EQ(dt.ymd, 20000229);
  EXPECT_NEAR(dt.hms, 0.0, 1e-6);

  dt = {19000228, 235959.0};
  TimeManagement::incrementDateTime(dt, 1.0);
  EXPECT_EQ(dt.ymd, 19000301);
  EXPECT_NEAR(dt.hms, 0.0, 1e-6);
}

TEST_F(TimeManagementTest, FractionalSeconds) {
  DateTime dt = {20240101, 120000.0};
  TimeManagement::incrementDateTime(dt, 0.5);
  EXPECT_NEAR(dt.hms, 120000.5, 1e-6);

  TimeManagement::incrementDateTime(dt, 0.7);
  EXPECT_NEAR(dt.hms, 120001.2, 1e-6);

  const DateTime dt1 = {20240101, 120000.0};
  const DateTime dt2 = {20240101, 120000.5};
  EXPECT_NEAR(TimeManagement::differenceInSeconds(dt1, dt2), 0.5, 1e-6);
}

TEST_F(TimeManagementTest, ThreeSixtyDayCalendar) {
  TimeManagement::setCalendarType(TimeManagement::CalendarType::ThreeSixtyDay);

  EXPECT_EQ(TimeManagement::incrementDateByDay(20240230, 1), 20240301);
  EXPECT_EQ(TimeManagement::incrementDateByDay(20241230, 1), 20250101);

  const DateTime dt1 = {20240101, 0.0};
  const DateTime dt2 = {20240201, 0.0};
  EXPECT_NEAR(TimeManagement::differenceInSeconds(dt1, dt2), 30 * 86400.0,
              1e-6);
}

TEST_F(TimeManagementTest, JulianConversions) {
  // Known Julian Day: 2024-02-26 is 2460367
  const int jd = TimeManagement::computeJulianDay(26, 2, 2024);
  EXPECT_EQ(jd, 2460367);

  int d, m, y;
  TimeManagement::computeCalendarDate(2460367, d, m, y);
  EXPECT_EQ(d, 26);
  EXPECT_EQ(m, 2);
  EXPECT_EQ(y, 2024);
}

TEST_F(TimeManagementTest, StringFormatting) {
  const DateTime dt = {20240226, 153045.5};
  EXPECT_EQ(TimeManagement::toFormattedString(dt), "2024/02/26 15:30:45 UTC");
  EXPECT_EQ(TimeManagement::toIsoString(dt), "2024-02-26T15:30:45");
}

TEST_F(TimeManagementTest, ArrayConversions) {
  const DateTime dt = {20240226, 153045.123};
  DateArray dat{};
  int ierr;
  TimeManagement::dateTimeToDateArray(dt, dat, ierr);
  EXPECT_EQ(ierr, 0);
  EXPECT_EQ(dat[0], 2024);
  EXPECT_EQ(dat[1], 2);
  EXPECT_EQ(dat[2], 26);
  EXPECT_EQ(dat[4], 15);
  EXPECT_EQ(dat[5], 30);
  EXPECT_EQ(dat[6], 45);
  EXPECT_EQ(dat[7], 123);

  DateTime dt2{};
  TimeManagement::dateArrayToDateTime(dat, dt2, ierr);
  EXPECT_EQ(dt2.ymd, dt.ymd);
  EXPECT_NEAR(dt2.hms, dt.hms, 0.001);
}

TEST_F(TimeManagementTest, SubtractionRoutines) {
  const DateArray t1 = {2024, 1, 1, 0, 0, 0, 0, 0};
  const DateArray t2 = {2024, 1, 2, 0, 12, 0, 0, 0}; // 1.5 days later

  EXPECT_NEAR(TimeManagement::differenceInDays(t1, t2), 1.5, 1e-6);
  EXPECT_NEAR(TimeManagement::differenceInSeconds(t1, t2), 1.5 * 86400.0, 1e-6);
}

TEST_F(TimeManagementTest, Profiling) {
  TimeManagement::initializeProfiling();
  const double t1 = TimeManagement::getProfilingTime();
  EXPECT_GE(t1, 0.0);
}

TEST_F(TimeManagementTest, UnitsConversion) {
  DateArray dat{};
  int ierr;
  TimeManagement::parseUnitsToDateArray("seconds since 1970-01-01 00:00:00",
                                        dat, ierr);
  EXPECT_EQ(ierr, 0);
  EXPECT_EQ(dat[0], 1970);
  EXPECT_EQ(dat[1], 1);
  EXPECT_EQ(dat[2], 1);

  TimeManagement::parseUnitsToDateArray("days since 2000-01-01T12:00:00", dat,
                                        ierr);
  EXPECT_EQ(ierr, 0);
  EXPECT_EQ(dat[0], 2000);
  EXPECT_EQ(dat[1], 1);
  EXPECT_EQ(dat[2], 1);
  EXPECT_EQ(dat[4], 12);
}

TEST_F(TimeManagementTest, SystemTimeRoutines) {
  DateArray dat{};
  TimeManagement::getSystemDateArray(dat);

  // Verify plausible date (2020+)
  EXPECT_GE(dat[0], 2020);
  EXPECT_LE(dat[0], 2100);
  EXPECT_GE(dat[1], 1);
  EXPECT_LE(dat[1], 12);
  EXPECT_GE(dat[2], 1);
  EXPECT_LE(dat[2], 31);

  // Test getElapsedTimeSince with a small sleep
  double elapsed;
  TimeManagement::getElapsedTimeSince(dat, elapsed);
  EXPECT_GE(elapsed, 0.0);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  TimeManagement::getElapsedTimeSince(dat, elapsed);
  EXPECT_GE(elapsed, 0.09); // Allow for slight jitter in sleep duration
  EXPECT_LT(elapsed, 1.0);  // Should be much less than 1s

  // Test getPresentDateTime
  const DateTime dt = TimeManagement::getPresentDateTime();
  EXPECT_GE(dt.ymd, 20200101);
}

TEST_F(TimeManagementTest, CalendarState) {
  // Check default (set to Standard in fixture SetUp)
  EXPECT_EQ(TimeManagement::getCalendarType(),
            TimeManagement::CalendarType::Standard);

  // Check manual setting
  TimeManagement::setCalendarType(TimeManagement::CalendarType::NoLeap);
  EXPECT_EQ(TimeManagement::getCalendarType(),
            TimeManagement::CalendarType::NoLeap);

  TimeManagement::setCalendarType(TimeManagement::CalendarType::ThreeSixtyDay);
  EXPECT_EQ(TimeManagement::getCalendarType(),
            TimeManagement::CalendarType::ThreeSixtyDay);

  TimeManagement::setCalendarType(TimeManagement::CalendarType::Standard);
  EXPECT_EQ(TimeManagement::getCalendarType(),
            TimeManagement::CalendarType::Standard);
}

TEST_F(TimeManagementTest, DayOfYear) {
  // Standard calendar
  EXPECT_EQ(TimeManagement::getDayOfYear(20230101), 1);
  EXPECT_EQ(TimeManagement::getDayOfYear(20230301), 60); // 31 + 28 + 1
  EXPECT_EQ(TimeManagement::getDayOfYear(20240301), 61); // 31 + 29 + 1 (Leap)

  // 360-day calendar
  TimeManagement::setCalendarType(TimeManagement::CalendarType::ThreeSixtyDay);
  EXPECT_EQ(TimeManagement::getDayOfYear(20240230), 60);  // 30 + 30
  EXPECT_EQ(TimeManagement::getDayOfYear(20241230), 360); // 12 * 30
}

TEST_F(TimeManagementTest, AdvancedJulianConversions) {
  // Standard calendar
  DateArray dat = {2000, 1, 1, 0, 12, 0, 0, 0};
  double julian;
  int errorCode;
  TimeManagement::dateArrayToJulianDay(dat, julian, errorCode);
  EXPECT_EQ(errorCode, 0);
  EXPECT_NEAR(julian, 2451545.0, 1e-6);

  // Round trip conversion
  DateArray dat2{};
  TimeManagement::julianDayToDateArray(julian, dat2, errorCode);
  EXPECT_EQ(errorCode, 0);
  for (size_t i = 0; i < 8; ++i) {
    if (i == 3)
      continue; // Skip timezone
    EXPECT_EQ(dat[i], dat2[i]);
  }

  // Error handling for year 0
  dat[0] = 0;
  TimeManagement::dateArrayToJulianDay(dat, julian, errorCode);
  EXPECT_EQ(errorCode, -1);

  // 360-day calendar
  TimeManagement::setCalendarType(TimeManagement::CalendarType::ThreeSixtyDay);
  dat = {1800, 1, 1, 0, 0, 0, 0, 0};
  TimeManagement::dateArrayToJulianDay(dat, julian, errorCode);
  EXPECT_EQ(errorCode, 0);
  EXPECT_NEAR(julian, 0.0, 1e-6);

  // Round trip
  TimeManagement::julianDayToDateArray(0.0, dat2, errorCode);
  EXPECT_EQ(errorCode, 0);
  for (size_t i = 0; i < 8; ++i) {
    if (i == 3)
      continue;
    EXPECT_EQ(dat[i], dat2[i]);
  }
}

TEST_F(TimeManagementTest, Time2Hours) {
  const DateTime dt = {20000101, 120000.0};
  const double hours = TimeManagement::time2hours(dt);
  // Julian day for 2000-01-01 is 2451545. 24 * 2451545 + 12 = 58837092.
  EXPECT_NEAR(hours, 58837092.0, 1e-6);
}

TEST_F(TimeManagementTest, NoLeapCalendar) {
  TimeManagement::setCalendarType(TimeManagement::CalendarType::NoLeap);

  // 2024 is a leap year in Standard, but not in NoLeap
  EXPECT_EQ(TimeManagement::incrementDateByDay(20240228, 1), 20240301);
  EXPECT_EQ(TimeManagement::getDayOfYear(20240301), 60); // 31 + 28 + 1
}

TEST_F(TimeManagementTest, NegativeIncrements) {
  DateTime dt = {20240101, 0.0};
  TimeManagement::incrementDateTime(dt, -1.0);
  EXPECT_EQ(dt.ymd, 20231231);
  EXPECT_NEAR(dt.hms, 235959.0, 1e-6);

  TimeManagement::incrementDateTime(dt, -86400.0);
  EXPECT_EQ(dt.ymd, 20231230);
  EXPECT_NEAR(dt.hms, 235959.0, 1e-6);
}

TEST_F(TimeManagementTest, YearZeroHandling) {
  EXPECT_EQ(TimeManagement::computeJulianDay(1, 1, 0), -1);

  DateArray dat = {0, 1, 1, 0, 0, 0, 0, 0};
  double julian;
  int errorCode;
  TimeManagement::dateArrayToJulianDay(dat, julian, errorCode);
  EXPECT_EQ(errorCode, -1);
}

TEST_F(TimeManagementTest, NotSetDateTime) {
  const DateTime dt = {-1, 0.0};
  EXPECT_EQ(TimeManagement::toFormattedString(dt), " date and time not set.");
}

TEST_F(TimeManagementTest, DifferenceInSecondsReverse) {
  const DateTime dt1 = {20240101, 120000.0};
  const DateTime dt2 = {20240101, 115959.0};
  EXPECT_NEAR(TimeManagement::differenceInSeconds(dt1, dt2), -1.0, 1e-6);

  const DateTime dt_midnight = {20240101, 0.0};
  const DateTime dt_prev_day = {20231231, 235959.0};
  EXPECT_NEAR(TimeManagement::differenceInSeconds(dt_midnight, dt_prev_day),
              -1.0, 1e-6);
}

TEST_F(TimeManagementTest, JulianErrorHandling) {
  DateArray dat{};
  int errorCode;
  // Standard calendar does not support negative Julian days
  TimeManagement::julianDayToDateArray(-1.0, dat, errorCode);
  EXPECT_EQ(errorCode, 1);
}

TEST_F(TimeManagementTest, UnitsParsingErrors) {
  DateArray dat{};
  int errorCode;
  TimeManagement::parseUnitsToDateArray("invalid units", dat, errorCode);
  EXPECT_EQ(errorCode, 1);

  TimeManagement::parseUnitsToDateArray("seconds since ", dat, errorCode);
  EXPECT_EQ(errorCode, 1);
}

TEST_F(TimeManagementTest, ResetTimeManagement) {
  TimeManagement::setCalendarType(TimeManagement::CalendarType::ThreeSixtyDay);
  EXPECT_EQ(TimeManagement::getCalendarType(),
            TimeManagement::CalendarType::ThreeSixtyDay);

  TimeManagement::reset();
  EXPECT_EQ(TimeManagement::getCalendarType(),
            TimeManagement::CalendarType::Standard);
}
