/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file w4core_init.h
 * @brief Initialization routine for the WAVEWATCH IV core.
 * @details This header defines the initialization routine for the WW4 core.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 * @note The architectural design of this routine follows the structure of
 *       the multi-grid shell (ww3_multi.F90) in WAVEWATCH III.
 *       Original author of WW3 multi-grid shell: Hendrik L. Tolman.
 */

#pragma once

#include "ww4_core/w4core_wave.h"
#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_input_utils.h"
#include "ww4_utils/ww4_run_config.h"
#include <fstream>
#include <iostream>

namespace ww4_core {

// --- w4core_init ------------------------------------------------------------
/**
 * @brief Initialization routine for the WAVEWATCH IV core.
 * @details Performs all necessary setup for the wave model core.
 * @param startTime Simulation start time.
 * @param programName Name of the program to identify in output banners.
 * @param os Output stream for reporting.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 */
void w4core_init(const ww4_utils::DateTime &startTime,
                 std::string_view programName, std::ostream &os);

// --- resetInternalState -----------------------------------------------------
/**
 * @brief Resets the internal state of the core module.
 * @details Clears global configuration, program name, and closes log file
 * stream.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 */
void resetInternalState() noexcept;

// --- Accessors --------------------------------------------------------------
/**
 * @brief Provides access to the captured program name.
 * @return A reference to the program name string.
 */
const std::string &getProgramName();

/**
 * @brief Provides access to the loaded run-time configuration.
 * @return A reference to the RunConfig structure.
 */
const ww4_utils::RunConfig &getRunConfig();

/**
 * @brief Provides access to the loaded run-time configuration for modification.
 * @return A mutable reference to the RunConfig structure.
 */
ww4_utils::RunConfig &getMutableRunConfig();

/**
 * @brief Provides access to the log file stream.
 * @return A reference to the std::ofstream for the log file.
 */
std::ofstream &getLogFileStream();

/**
 * @brief Provides access to the wave time data.
 * @return A reference to the waveTimeData structure.
 */
const ww4_utils::waveTimeData &getWaveTimeData();

/**
 * @brief Updates the model time in the wave time data.
 * @param time The new model time.
 */
void updateWaveModelTime(const ww4_utils::DateTime &time);

/**
 * @brief Updates the input time data for a specific input type.
 * @param type The input type to update.
 * @param data The new input time data.
 */
void updateWaveInputTime(ww4_utils::InputType type,
                         const ww4_utils::intTimeData &data);

} // namespace ww4_core
