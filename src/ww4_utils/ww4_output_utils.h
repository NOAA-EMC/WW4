/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_output_utils.h
 * @brief Utility structures and routines for model output processing.
 * @details This header defines the OutputConfig structure and routines for
 *          managing model output time data.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Hendrik L. Tolman, Aldgisl (AI Persona)
 * @author Contributors: Jules (Agentic AI)
 * @date 2026-05-01
 */

#pragma once

#include "ww4_utils/time_management.h"
#include <optional>

namespace ww4_utils {

// Forward declaration of RunConfig to avoid circular dependency
struct RunConfig;

/**
 * @struct OutputConfig
 * @brief Configuration for a specific output type.
 * @author Main Author(s): Hendrik L. Tolman, Aldgisl (AI Persona)
 * @author Contributors: Jules (Agentic AI)
 * @var OutputConfig::requested
 * @brief Is this output requested?
 * @var OutputConfig::startTime
 * @brief Start time for output.
 * @var OutputConfig::endTime
 * @brief End time for output.
 * @var OutputConfig::interval
 * @brief Output interval in seconds.
 * @var OutputConfig::atFirstTime
 * @brief Output at first time?
 * @var OutputConfig::actualTime
 * @brief Next scheduled output time.
 */
struct OutputConfig {
  bool requested = false;
  std::optional<DateTime> startTime;
  std::optional<DateTime> endTime;
  double interval = -1.0;
  bool atFirstTime = true;
  std::optional<DateTime> actualTime;
};

void assessOutputConfig(const DateTime &modelTime, const DateTime &endTime,
                        RunConfig &config);

double computeOutputTimeStep(const DateTime &modelTime, const DateTime &endTime,
                             const RunConfig &config);

void updateOutputActualTimes(const DateTime &modelTime, const DateTime &endTime,
                             RunConfig &config);

} // namespace ww4_utils
