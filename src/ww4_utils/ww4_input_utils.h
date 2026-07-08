/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_input_utils.h
 * @brief Utility structures and routines for model input processing.
 * @details This header defines structures and routines for managing model
 *          input time data and orchestration of input updates.
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

#pragma once

#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_logfile.h"
#include "ww4_utils/ww4_run_config.h"
#include <iostream>
#include <optional>
#include <vector>

namespace ww4_utils {

/**
 * @enum InputType
 * @brief Types of model inputs for time management.
 * @var InputType::WaterLevels
 * @brief Water levels input.
 * @var InputType::Currents
 * @brief Currents input.
 * @var InputType::Winds
 * @brief Winds input.
 * @var InputType::IceConcentrations
 * @brief Ice concentrations input.
 * @var InputType::BottomDepth
 * @brief Bottom depth input.
 */
enum class InputType {
  WaterLevels,
  Currents,
  Winds,
  IceConcentrations,
  BottomDepth,
};

/**
 * @struct intTimeData
 * @brief Structure to hold time tags for model inputs.
 * @var intTimeData::time1
 * @brief First time tag.
 * @var intTimeData::time2
 * @brief Second time tag.
 * @var intTimeData::maxStep
 * @brief Maximum model time step.
 */
struct intTimeData {
  std::optional<DateTime> time1;
  std::optional<DateTime> time2;
  double maxStep = -1.0;
};

/**
 * @struct waveTimeData
 * @brief Structure to hold model time and time step information.
 * @var waveTimeData::timeStep
 * @brief Model time step.
 * @var waveTimeData::modelTime
 * @brief Current model time.
 * @var waveTimeData::waterLevels
 * @brief Time data for water levels.
 * @var waveTimeData::currents
 * @brief Time data for currents.
 * @var waveTimeData::winds
 * @brief Time data for winds.
 * @var waveTimeData::iceConcentrations
 * @brief Time data for ice concentrations.
 * @var waveTimeData::bottomDepth
 * @brief Time data for bottom depth.
 */
struct waveTimeData {
  double timeStep = -1.0;
  std::optional<DateTime> modelTime;
  intTimeData waterLevels;
  intTimeData currents;
  intTimeData winds;
  intTimeData iceConcentrations;
  intTimeData bottomDepth;
};

/**
 * @struct InputUpdateState
 * @brief Tracking state for input interpolation reporting.
 */
struct InputUpdateState {
  std::optional<DateTime> lastWlTime1, lastWlTime2;
  std::optional<DateTime> lastCuTime1, lastCuTime2;
  std::optional<DateTime> lastWiTime1, lastWiTime2;
  std::optional<DateTime> lastIcTime1, lastIcTime2;
  std::optional<DateTime> lastBdTime1, lastBdTime2;
};

void ww4_input_update(const RunConfig &config, std::ostream &os);

void resetInputData() noexcept;

const std::vector<HomogeneousDataPoint> &getHomogeneousWaterLevels() noexcept;

const std::vector<HomogeneousDataPoint> &getHomogeneousCurrents() noexcept;

const std::vector<HomogeneousDataPoint> &getHomogeneousWinds() noexcept;

const std::vector<HomogeneousDataPoint> &
getHomogeneousIceConcentrations() noexcept;

const std::vector<HomogeneousDataPoint> &getHomogeneousBottomDepth() noexcept;

void ww4_hom_water_levels(const DateTime &modelTime, const DateTime &endTime,
                          intTimeData &data);

void ww4_hom_currents(const DateTime &modelTime, const DateTime &endTime,
                      intTimeData &data);

void ww4_hom_winds(const DateTime &modelTime, const DateTime &endTime,
                   intTimeData &data);

void ww4_hom_ice(const DateTime &modelTime, const DateTime &endTime,
                 intTimeData &data);

void ww4_hom_bottom_depth(const DateTime &modelTime, const DateTime &endTime,
                          intTimeData &data);

double updateAllInputs(const DateTime &modelTime, const DateTime &endTime,
                       waveTimeData &waveTime, InputUpdateState &state,
                       const RunConfig &config, bool &headerPrinted,
                       std::ostream &os, ww4_logfile::LogTableData &logData);

double computeInputTimeStep(const DateTime &modelTime, const DateTime &endTime,
                            const waveTimeData &waveTime,
                            const RunConfig &config);

} // namespace ww4_utils
