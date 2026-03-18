/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file test_time_management.cpp
 * @brief Unit tests for time management routines using Google Test.
 * @details This file provides comprehensive unit testing for the TimeManagement
 * class, covering leap year cycles, fractional seconds, different calendars,
 *          and conversions.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * @author Hendrik L. Tolman (Initial, 2026-02-27)
 * @author Aldgisl, Hendrik L. Tolman (Last Update, 2026-03-18)
 * @date 2026-03-18
 */

#include "ww4_utils/time_management.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <vector>

using namespace ww4_utils;

/**
 * @class TimeManagementTest
 * @brief Test fixture for TimeManagement tests.
 */
class TimeManagementTest : public ::testing::Test {
protected:
  /**
   * @brief Sets up the test environment by setting the calendar to Standard.
   */
  void SetUp() override {
    TimeManagement::setCalendarType(TimeManagement::CalendarType::Standard);
  }
};

/**
 * @test Verify leap year handling across multiple cycles.
 */
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

/**
 * @test Verify fractional second arithmetic.
 */
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

/**
 * @test Verify 360-day calendar behavior.
 */
TEST_F(TimeManagementTest, ThreeSixtyDayCalendar) {
  TimeManagement::setCalendarType(TimeManagement::CalendarType::ThreeSixtyDay);

  EXPECT_EQ(TimeManagement::incrementDateByDay(20240230, 1), 20240301);
  EXPECT_EQ(TimeManagement::incrementDateByDay(20241230, 1), 20250101);

  const DateTime dt1 = {20240101, 0.0};
  const DateTime dt2 = {20240201, 0.0};
  EXPECT_NEAR(TimeManagement::differenceInSeconds(dt1, dt2), 30 * 86400.0,
              1e-6);
}

/**
 * @test Verify Julian day conversions.
 */
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

/**
 * @test Verify string formatting routines.
 */
TEST_F(TimeManagementTest, StringFormatting) {
  const DateTime dt = {20240226, 153045.5};
  EXPECT_EQ(TimeManagement::toFormattedString(dt), "2024/02/26 15:30:45 UTC");
  EXPECT_EQ(TimeManagement::toIsoString(dt), "2024-02-26T15:30:45");
}

/**
 * @test Verify conversions between DateTime and DateArray.
 */
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

/**
 * @test Verify subtraction of dates and times.
 */
TEST_F(TimeManagementTest, SubtractionRoutines) {
  const DateArray t1 = {2024, 1, 1, 0, 0, 0, 0, 0};
  const DateArray t2 = {2024, 1, 2, 0, 12, 0, 0, 0}; // 1.5 days later

  EXPECT_NEAR(TimeManagement::differenceInDays(t1, t2), 1.5, 1e-6);
  EXPECT_NEAR(TimeManagement::differenceInSeconds(t1, t2), 1.5 * 86400.0, 1e-6);
}

/**
 * @test Verify basic profiling initialization and capture.
 */
TEST_F(TimeManagementTest, Profiling) {
  TimeManagement::initializeProfiling();
  const double t1 = TimeManagement::getProfilingTime();
  EXPECT_GE(t1, 0.0);
}

/**
 * @test Verify parsing of time units strings.
 */
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

/**
 * @test Verify system time retrieval and elapsed time calculation.
 */
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
