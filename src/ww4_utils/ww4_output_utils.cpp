/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_output_utils.cpp
 * @brief Implementation of model output orchestration routines.
 * @details This file implements routines for managing model output time data.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Hendrik L. Tolman, Aldgisl (AI Persona)
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-05-01
 * @date Last update : 2026-07-07
 */

#include "ww4_utils/ww4_output_utils.h"
#include "ww4_utils/ww4_run_config.h"
#include <algorithm>

/**
 * @namespace ww4_utils
 * @brief Utilities for WAVEWATCH IV.
 */
namespace ww4_utils {

/**
 * @brief Assesses and initializes output configurations.
 * @param modelTime Current model time.
 * @param endTime Simulation end time.
 * @param config Run configuration to update.
 */
void assessOutputConfig(const DateTime &modelTime, const DateTime &endTime,
                        RunConfig &config) {
  auto assess = [&](OutputConfig &oc, bool isApi) {
    if (!oc.requested)
      return;

    if (!oc.startTime) {
      oc.startTime = modelTime;
    }

    if (isApi) {
      oc.actualTime = endTime;
    } else {
      if (oc.atFirstTime) {
        oc.actualTime = oc.startTime;
      } else {
        oc.actualTime = oc.startTime;
        TimeManagement::incrementDateTime(*oc.actualTime, oc.interval);
      }
    }

    // Deactivate if past endTime
    if (oc.actualTime.has_value()) {
      if (TimeManagement::differenceInSeconds(*oc.actualTime, endTime) <
          -0.001) {
        oc.requested = false;
      } else if (oc.endTime.has_value() &&
                 TimeManagement::differenceInSeconds(*oc.actualTime,
                                                     *oc.endTime) < -0.001) {
        oc.requested = false;
      }
    }
  };

  assess(config.outputFields, false);
  assess(config.outputPoints, false);
  assess(config.outputRestart, false);
  assess(config.outputApi, true);
}

/**
 * @brief Computes the minimum time step to the next requested output.
 * @param modelTime Current model time.
 * @param endTime Simulation end time.
 * @param config Run configuration.
 * @return Minimum time step in seconds.
 */
double computeOutputTimeStep(const DateTime &modelTime, const DateTime &endTime,
                             const RunConfig &config) {
  double minStep = TimeManagement::differenceInSeconds(modelTime, endTime);

  auto check = [&](const OutputConfig &oc) {
    if (oc.requested && oc.actualTime.has_value()) {
      double step =
          TimeManagement::differenceInSeconds(modelTime, *oc.actualTime);
      if (step >= 0.0) {
        minStep = std::min(minStep, step);
      }
    }
  };

  check(config.outputFields);
  check(config.outputPoints);
  check(config.outputRestart);
  check(config.outputApi);

  return std::max(0.0, minStep);
}

/**
 * @brief Updates actual output times after output has been performed.
 * @param modelTime Current model time.
 * @param endTime Simulation end time.
 * @param config Run configuration to update.
 */
void updateOutputActualTimes(const DateTime &modelTime, const DateTime &endTime,
                             RunConfig &config) {
  auto update = [&](OutputConfig &oc) {
    if (!oc.requested || !oc.actualTime.has_value())
      return;

    if (*oc.actualTime == modelTime) {
      TimeManagement::incrementDateTime(*oc.actualTime, oc.interval);
    }

    // Deactivate if past endTime
    if (TimeManagement::differenceInSeconds(*oc.actualTime, endTime) < -0.001) {
      oc.requested = false;
    } else if (oc.endTime.has_value() &&
               TimeManagement::differenceInSeconds(*oc.actualTime,
                                                   *oc.endTime) < -0.001) {
      oc.requested = false;
    }
  };

  update(config.outputFields);
  update(config.outputPoints);
  update(config.outputRestart);
  // API output is typically one-shot at the end, interval is -1.0 so update
  // will just pass.
  update(config.outputApi);
}

} // namespace ww4_utils
