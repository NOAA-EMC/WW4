/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_input_utils.cpp
 * @brief Implementation of model input orchestration routines.
 * @details This file implements the updateAllInputs routine and input data
 *          processing for WW4.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-21
 * @date Last update : 2026-07-07
 */

#include "ww4_utils/ww4_input_utils.h"
#include "ww4_utils/ww4_logfile.h"
#include "ww4_utils/ww4_run_config.h"
#include "ww4_utils/ww4_std_out.h"
#include <algorithm>
#include <iostream>
#include <vector>

/**
 * @namespace ww4_utils
 * @brief Utilities for WAVEWATCH IV.
 */
namespace ww4_utils {

namespace {

std::vector<HomogeneousDataPoint> waterLevels;
std::vector<HomogeneousDataPoint> currents;
std::vector<HomogeneousDataPoint> winds;
std::vector<HomogeneousDataPoint> iceConcentrations;
std::vector<HomogeneousDataPoint> bottomDepth;

/**
 * @brief Helper to validate and process a data series.
 * @param source Vector of pre-parsed data points from RunConfig.
 * @param processed Vector to store validated data points.
 * @param fieldName Name of the field for error reporting.
 * @param option The input option for this field.
 * @param os Output stream for reporting.
 */
void processSeries(const std::vector<HomogeneousDataPoint> &source,
                   std::vector<HomogeneousDataPoint> &processed,
                   std::string_view fieldName,
                   ww4_utils::InputFieldOption option, std::ostream &os) {

  if (option != ww4_utils::InputFieldOption::Homogeneous) {
    return;
  }

  if (source.empty()) {
    ww4_utils::ww4_std_out::extcde(1, os,
                                   "No data provided for homogeneous field: " +
                                       std::string(fieldName),
                                   __FILE__, __LINE__);
  }

  for (const auto &dp : source) {
    if (!processed.empty()) {
      const double diff = ww4_utils::TimeManagement::differenceInSeconds(
          processed.back().time, dp.time);
      if (diff < 0.0) {
        ww4_utils::ww4_std_out::extcde(1, os,
                                       "Time stamps go backward in data for " +
                                           std::string(fieldName),
                                       __FILE__, __LINE__);
      }
    }

    // Field-specific validation
    if (fieldName == "water levels" || fieldName == "bottom depth") {
      if (dp.values.size() != 1) {
        ww4_utils::ww4_std_out::extcde(1, os,
                                       "Homogeneous " + std::string(fieldName) +
                                           " requires 1 value.",
                                       __FILE__, __LINE__);
      }
    } else if (fieldName == "currents") {
      if (dp.values.size() != 2) {
        ww4_utils::ww4_std_out::extcde(
            1, os, "Homogeneous currents requires 2 values (speed, direction).",
            __FILE__, __LINE__);
      }
    } else if (fieldName == "winds") {
      if (dp.values.size() < 2 || dp.values.size() > 3) {
        ww4_utils::ww4_std_out::extcde(1, os,
                                       "Homogeneous winds requires 2 or 3 "
                                       "values (speed, direction, [temp]).",
                                       __FILE__, __LINE__);
      }
    } else if (fieldName == "ice concentrations") {
      if (dp.values.size() != 1) {
        ww4_utils::ww4_std_out::extcde(1, os,
                                       "Homogeneous ice concentrations "
                                       "requires 1 value.",
                                       __FILE__, __LINE__);
      }
      if (dp.values[0] < 0.0 || dp.values[0] > 1.0) {
        ww4_utils::ww4_std_out::extcde(
            1, os, "Ice concentration must be between 0.0 and 1.0.", __FILE__,
            __LINE__);
      }
    }

    processed.push_back(dp);
  }
}

/**
 * @brief Helper to cycle through homogeneous input data.
 * @param series The vector of homogeneous data points.
 * @param modelTime Current model time.
 * @param endTime Simulation end time for capping max step.
 * @param data Output structure to store interpolation interval and max step.
 */
void updateHomogeneousInputCycling(
    const std::vector<HomogeneousDataPoint> &series, const DateTime &modelTime,
    const DateTime &endTime, intTimeData &data) {

  if (series.empty()) {
    data.time1 = modelTime;
    data.time2 = endTime;
    data.maxStep = TimeManagement::differenceInSeconds(modelTime, endTime);
    return;
  }

  // Find the interval around the present model time.
  // The first and second time tags should be around the present model time,
  // where the first time tag can be equal to the model time.

  // Case 1: If the first time of the homogeneous input is after the present
  // model time, set the first time to the present model time, and the second
  // time to the first time for which the homogeneous input is defined.
  if (TimeManagement::differenceInSeconds(modelTime, series.front().time) >
      0.001) {
    data.time1 = modelTime;
    data.time2 = series.front().time;
  }
  // Case 2: If the last time for the input is before the model time,
  // set the second time to the ending time of the run.
  // (time1 will be the last data point time).
  else if (TimeManagement::differenceInSeconds(series.back().time, modelTime) >
           -0.001) {
    data.time1 = series.back().time;
    data.time2 = endTime;
  }
  // Case 3: Model time is within the range of the input data.
  else {
    for (size_t i = 0; i < series.size() - 1; ++i) {
      if (TimeManagement::differenceInSeconds(series[i].time, modelTime) >=
              -0.001 &&
          TimeManagement::differenceInSeconds(modelTime, series[i + 1].time) >
              0.001) {
        data.time1 = series[i].time;
        data.time2 = series[i + 1].time;
        break;
      }
    }
  }

  // Ensure time2 is not before modelTime
  if (data.time2.has_value() &&
      TimeManagement::differenceInSeconds(modelTime, *data.time2) < 0.0) {
    data.time2 = endTime;
  }

  // Calculate maxStep: the time interval from the present model time to the
  // second time tag.
  if (data.time2.has_value()) {
    data.maxStep = TimeManagement::differenceInSeconds(modelTime, *data.time2);
  } else {
    data.maxStep = TimeManagement::differenceInSeconds(modelTime, endTime);
    data.time2 = endTime;
  }
}

/**
 * @brief Helper to process a single input field.
 * @param[in] fieldName Name of the field for logging.
 * @param[in] option Input option for this field.
 * @param[in] modelTime Current model time.
 * @param[in] endTime Simulation end time.
 * @param[in,out] data Data structure for this field.
 * @param[in,out] lastTime1 Tracker for last reported time1.
 * @param[in,out] lastTime2 Tracker for last reported time2.
 * @param[in] config The run configuration.
 * @param[in,out] headerPrinted Flag to track if the step header was printed.
 * @param[in,out] os Standard output stream.
 * @param[in,out] logData Data for tabular log output.
 */
void processField(std::string_view fieldName, InputFieldOption option,
                  const DateTime &modelTime, const DateTime &endTime,
                  intTimeData &data, std::optional<DateTime> &lastTime1,
                  std::optional<DateTime> &lastTime2, const RunConfig &config,
                  bool &headerPrinted, std::ostream &os,
                  ww4_logfile::LogTableData &logData) {

  if (option == InputFieldOption::None ||
      option == InputFieldOption::Undefined) {
    return;
  }

  if (option == InputFieldOption::Homogeneous) {
    if (fieldName == "water levels") {
      ww4_hom_water_levels(modelTime, endTime, data);
    } else if (fieldName == "currents") {
      ww4_hom_currents(modelTime, endTime, data);
    } else if (fieldName == "winds") {
      ww4_hom_winds(modelTime, endTime, data);
    } else if (fieldName == "ice concentrations") {
      ww4_hom_ice(modelTime, endTime, data);
    } else if (fieldName == "bottom depth") {
      ww4_hom_bottom_depth(modelTime, endTime, data);
    }

    if (data.time1.has_value() && data.time2.has_value()) {
      if (data.time1 != lastTime1 || data.time2 != lastTime2) {
        if (config.produceStdOut &&
            config.screenOutputLevel != ScreenOutputLevel::None) {
          if (config.screenOutputLevel == ScreenOutputLevel::Summary &&
              !headerPrinted) {
            os << "  Computation step starting "
               << TimeManagement::toFormattedString(modelTime) << std::endl;
            headerPrinted = true;
          }
          if (headerPrinted) {
            ww4_std_out::writeUpdatingField(os, fieldName);
            ww4_std_out::writeInterpolationInfo(os, *data.time1, *data.time2);
          }
        }
        if (config.produceLogFile) {
          if (fieldName == "water levels")
            logData.wlUpdated = true;
          else if (fieldName == "currents")
            logData.cuUpdated = true;
          else if (fieldName == "winds")
            logData.wiUpdated = true;
          else if (fieldName == "ice concentrations")
            logData.icUpdated = true;
          else if (fieldName == "bottom depth")
            logData.bdUpdated = true;
        }
        lastTime1 = data.time1;
        lastTime2 = data.time2;
      }
    }
  }
}

} // namespace

/**
 * @brief Processes and validates input data for the model.
 * @details Parses input data from the run configuration and
 *          validates its availability and temporal consistency.
 * @param config The run configuration.
 * @param os Output stream for reporting.
 * @date 2026-05-01
 */
void ww4_input_update(const RunConfig &config, std::ostream &os) {
  // Reset before processing
  resetInputData();

  processSeries(config.homogeneousWaterLevels, waterLevels, "water levels",
                config.waterLevels, os);
  processSeries(config.homogeneousCurrents, currents, "currents",
                config.currents, os);
  processSeries(config.homogeneousWinds, winds, "winds", config.winds, os);
  processSeries(config.homogeneousIceConcentrations, iceConcentrations,
                "ice concentrations", config.iceConcentrations, os);
  processSeries(config.homogeneousBottomDepth, bottomDepth, "bottom depth",
                config.bottomDepth, os);
}

/**
 * @brief Resets all internal input data storage.
 * @details Clears vectors containing processed homogeneous data.
 */
void resetInputData() noexcept {
  waterLevels.clear();
  currents.clear();
  winds.clear();
  iceConcentrations.clear();
  bottomDepth.clear();
}

/**
 * @brief Accessor for processed homogeneous water levels.
 * @return Reference to the vector of data points.
 */
const std::vector<HomogeneousDataPoint> &getHomogeneousWaterLevels() noexcept {
  return waterLevels;
}

/**
 * @brief Accessor for processed homogeneous currents.
 * @return Reference to the vector of data points.
 */
const std::vector<HomogeneousDataPoint> &getHomogeneousCurrents() noexcept {
  return currents;
}

/**
 * @brief Accessor for processed homogeneous winds.
 * @return Reference to the vector of data points.
 */
const std::vector<HomogeneousDataPoint> &getHomogeneousWinds() noexcept {
  return winds;
}

/**
 * @brief Accessor for processed homogeneous ice concentrations.
 * @return Reference to the vector of data points.
 */
const std::vector<HomogeneousDataPoint> &
getHomogeneousIceConcentrations() noexcept {
  return iceConcentrations;
}

/**
 * @brief Accessor for processed homogeneous bottom depth.
 * @return Reference to the vector of data points.
 */
const std::vector<HomogeneousDataPoint> &getHomogeneousBottomDepth() noexcept {
  return bottomDepth;
}

/**
 * @brief Cycle through homogeneous water levels to find interpolation interval.
 * @param modelTime Current model time.
 * @param endTime Simulation end time for capping max step.
 * @param data Output structure to store interpolation interval and max step.
 */
void ww4_hom_water_levels(const DateTime &modelTime, const DateTime &endTime,
                          intTimeData &data) {
  updateHomogeneousInputCycling(waterLevels, modelTime, endTime, data);
}

/**
 * @brief Cycle through homogeneous currents to find interpolation interval.
 * @param modelTime Current model time.
 * @param endTime Simulation end time for capping max step.
 * @param data Output structure to store interpolation interval and max step.
 */
void ww4_hom_currents(const DateTime &modelTime, const DateTime &endTime,
                      intTimeData &data) {
  updateHomogeneousInputCycling(currents, modelTime, endTime, data);
}

/**
 * @brief Cycle through homogeneous winds to find interpolation interval.
 * @param modelTime Current model time.
 * @param endTime Simulation end time for capping max step.
 * @param data Output structure to store interpolation interval and max step.
 */
void ww4_hom_winds(const DateTime &modelTime, const DateTime &endTime,
                   intTimeData &data) {
  updateHomogeneousInputCycling(winds, modelTime, endTime, data);
}

/**
 * @brief Cycle through homogeneous ice concentrations to find interpolation
 * interval.
 * @param modelTime Current model time.
 * @param endTime Simulation end time for capping max step.
 * @param data Output structure to store interpolation interval and max step.
 */
void ww4_hom_ice(const DateTime &modelTime, const DateTime &endTime,
                 intTimeData &data) {
  updateHomogeneousInputCycling(iceConcentrations, modelTime, endTime, data);
}

/**
 * @brief Cycle through homogeneous bottom depth to find interpolation interval.
 * @param modelTime Current model time.
 * @param endTime Simulation end time for capping max step.
 * @param data Output structure to store interpolation interval and max step.
 */
void ww4_hom_bottom_depth(const DateTime &modelTime, const DateTime &endTime,
                          intTimeData &data) {
  updateHomogeneousInputCycling(bottomDepth, modelTime, endTime, data);
}

/**
 * @brief Processes all input fields and calculates next time step.
 * @details Orchestrates the update of all active input fields and returns
 *          the time interval to the next required update.
 * @param[in] modelTime Current model time.
 * @param[in] endTime Simulation end time.
 * @param[in,out] waveTime Global wave time data to update.
 * @param[in,out] state Persistent state for reporting interpolation intervals.
 * @param[in] config The run configuration.
 * @param[in,out] headerPrinted Flag to track if the step header was printed.
 * @param[in] os Output stream for reporting.
 * @param[in,out] logData Data for tabular log output.
 * @return The time step (seconds) from the present model time to the next
 * update.
 */
double updateAllInputs(const DateTime &modelTime, const DateTime &endTime,
                       waveTimeData &waveTime, InputUpdateState &state,
                       const RunConfig &config, bool &headerPrinted,
                       std::ostream &os, ww4_logfile::LogTableData &logData) {

  processField("water levels", config.waterLevels, modelTime, endTime,
               waveTime.waterLevels, state.lastWlTime1, state.lastWlTime2,
               config, headerPrinted, os, logData);

  processField("currents", config.currents, modelTime, endTime,
               waveTime.currents, state.lastCuTime1, state.lastCuTime2, config,
               headerPrinted, os, logData);

  processField("winds", config.winds, modelTime, endTime, waveTime.winds,
               state.lastWiTime1, state.lastWiTime2, config, headerPrinted, os,
               logData);

  processField("ice concentrations", config.iceConcentrations, modelTime,
               endTime, waveTime.iceConcentrations, state.lastIcTime1,
               state.lastIcTime2, config, headerPrinted, os, logData);

  processField("bottom depth", config.bottomDepth, modelTime, endTime,
               waveTime.bottomDepth, state.lastBdTime1, state.lastBdTime2,
               config, headerPrinted, os, logData);

  return computeInputTimeStep(modelTime, endTime, waveTime, config);
}

/**
 * @brief Computes the minimum input time step based on active fields.
 * @param modelTime Current model time.
 * @param endTime Simulation end time.
 * @param waveTime Global wave time data.
 * @param config The run configuration.
 * @return The minimum required time step (seconds).
 */
double computeInputTimeStep(const DateTime &modelTime, const DateTime &endTime,
                            const waveTimeData &waveTime,
                            const RunConfig &config) {
  // Calculate minimum maxStep for inputs only
  double inputTimeStep =
      TimeManagement::differenceInSeconds(modelTime, endTime);

  if (config.waterLevels != InputFieldOption::None &&
      config.waterLevels != InputFieldOption::Undefined) {
    if (waveTime.waterLevels.maxStep > 0.0) {
      inputTimeStep = std::min(inputTimeStep, waveTime.waterLevels.maxStep);
    }
  }

  if (config.currents != InputFieldOption::None &&
      config.currents != InputFieldOption::Undefined) {
    if (waveTime.currents.maxStep > 0.0) {
      inputTimeStep = std::min(inputTimeStep, waveTime.currents.maxStep);
    }
  }

  if (config.winds != InputFieldOption::None &&
      config.winds != InputFieldOption::Undefined) {
    if (waveTime.winds.maxStep > 0.0) {
      inputTimeStep = std::min(inputTimeStep, waveTime.winds.maxStep);
    }
  }

  if (config.iceConcentrations != InputFieldOption::None &&
      config.iceConcentrations != InputFieldOption::Undefined) {
    if (waveTime.iceConcentrations.maxStep > 0.0) {
      inputTimeStep =
          std::min(inputTimeStep, waveTime.iceConcentrations.maxStep);
    }
  }

  if (config.bottomDepth != InputFieldOption::None &&
      config.bottomDepth != InputFieldOption::Undefined) {
    if (waveTime.bottomDepth.maxStep > 0.0) {
      inputTimeStep = std::min(inputTimeStep, waveTime.bottomDepth.maxStep);
    }
  }

  if (inputTimeStep < 0.0) {
    inputTimeStep = 0.0;
  }

  return inputTimeStep;
}

} // namespace ww4_utils
