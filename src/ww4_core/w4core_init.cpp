/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file w4core_init.cpp
 * @brief Implementation of the WAVEWATCH IV initialization routine.
 * @details This file implements the w4core_init routine.
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

#include "ww4_core/w4core_init.h"
#include "ww4_core/solver_smc/solver_smc.h"
#include "ww4_core/solver_triangular/solver_triangular.h"
#include "ww4_core/solver_uq/solver_uq.h"
#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_input_utils.h"
#include "ww4_utils/ww4_logfile.h"
#include "ww4_utils/ww4_run_config.h"
#include "ww4_utils/ww4_std_out.h"
#include <exception>
#include <fstream>
#include <iostream>

namespace ww4_core {

namespace {
ww4_utils::RunConfig globalRunConfig;
std::ofstream logFile;
std::string capturedProgramName;
ww4_utils::waveTimeData waveTimeData;
} // namespace

// --- w4core_init ------------------------------------------------------------
/**
 * @brief Initialization routine for the WAVEWATCH IV core.
 * @details Performs all necessary setup for the wave model core.
 *          Follows the architectural design of the initialization in
 *          ww3_multi.F90 from WAVEWATCH III.
 * @param startTime Simulation start time.
 * @param programName Name of the program to identify in output banners.
 * @param os Output stream for reporting.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-25
 */
void w4core_init(const ww4_utils::DateTime &startTime,
                 std::string_view programName, std::ostream &os) {
  try {
    // === General initialization =============================================
    capturedProgramName = std::string(programName);

    // === Load run-time configuration =======================================
    const auto config = ww4_utils::loadRunConfig("ww4_run_config.yaml", os);
    if (!config) {
      // Explanatory comment preceding __FILE__ and __LINE__
      // Exit program when configuration file loading fails
      ww4_utils::ww4_std_out::extcde(1, os,
                                     "Run-time configuration file "
                                     "'ww4_run_config.yaml' not found or could "
                                     "not be opened.",
                                     __FILE__, __LINE__);
    }
    globalRunConfig = *config;

    // === Calendar, profiling, and inputs setup ==============================
    ww4_utils::TimeManagement::setCalendarType(globalRunConfig.calendarType);
    ww4_utils::TimeManagement::initializeProfiling();
    ww4_utils::ww4_input_update(globalRunConfig, os);

    // === Initial standard output ============================================
    if (globalRunConfig.produceStdOut) {
      ww4_utils::ww4_std_out::writeInitialOutput(os, capturedProgramName);

      os << "  Run starts at "
         << ww4_utils::TimeManagement::toFormattedString(
                ww4_utils::TimeManagement::getPresentDateTime())
         << std::endl;

      os << "\n  Initialization (w4core_init) starting: "
         << ww4_utils::TimeManagement::toFormattedString(startTime)
         << std::endl;

      ww4_utils::reportRunConfig(globalRunConfig, os);
    }

    // === Initial log file output ============================================
    if (globalRunConfig.produceLogFile) {
      logFile.open("ww4_log.txt");
      ww4_utils::ww4_logfile::writeInitialOutput(logFile, capturedProgramName);

      logFile << "  Run starts at "
              << ww4_utils::TimeManagement::toFormattedString(
                     ww4_utils::TimeManagement::getPresentDateTime())
              << std::endl;

      logFile << "\n  Initialization (w4core_init) starting: "
              << ww4_utils::TimeManagement::toFormattedString(startTime)
              << std::endl;

      ww4_utils::reportRunConfig(globalRunConfig, logFile);
      ww4_utils::ww4_logfile::writeLogTableHeader(logFile);
    }

  } catch (const std::exception &e) {
    // Explanatory comment preceding __FILE__ and __LINE__
    // Terminate execution on standard exception
    ww4_utils::ww4_std_out::extcde(1, os, e.what(), __FILE__, __LINE__);
  } catch (...) {
    // Explanatory comment preceding __FILE__ and __LINE__
    // Terminate execution on unknown exception
    ww4_utils::ww4_std_out::extcde(1, os, "Unknown exception in w4core_init",
                                   __FILE__, __LINE__);
  }

  // === Initialize numerical solver ==========================================
  switch (globalRunConfig.solver) {
  case ww4_utils::SolverType::UQ:
    w4core_init_uq(os);
    break;
  case ww4_utils::SolverType::Triangular:
    w4core_init_triangular(os);
    break;
  case ww4_utils::SolverType::SMC:
    w4core_init_smc(os);
    break;
  default:
    // Explanatory comment preceding __FILE__ and __LINE__
    // Terminate execution on invalid solver configuration
    ww4_utils::ww4_std_out::extcde(
        1, os, "No numerical solver specified or unknown solver.", __FILE__,
        __LINE__);
    break;
  }

  // === Data initialization ==================================================
  waveTimeData.modelTime = startTime;
  waveTimeData.timeStep = globalRunConfig.timeStep;
}

// --- Accessors --------------------------------------------------------------

/**
 * @brief Provides access to the loaded run-time configuration.
 * @return A reference to the RunConfig structure.
 */
const ww4_utils::RunConfig &getRunConfig() { return globalRunConfig; }

/**
 * @brief Provides access to the loaded run-time configuration for modification.
 * @return A mutable reference to the RunConfig structure.
 */
ww4_utils::RunConfig &getMutableRunConfig() { return globalRunConfig; }

/**
 * @brief Provides access to the log file stream.
 * @return A reference to the std::ofstream for the log file.
 */
std::ofstream &getLogFileStream() { return logFile; }

/**
 * @brief Provides access to the captured program name.
 * @return A reference to the program name string.
 */
const std::string &getProgramName() { return capturedProgramName; }

/**
 * @brief Provides access to the wave time data.
 * @return A reference to the waveTimeData structure.
 */
const ww4_utils::waveTimeData &getWaveTimeData() { return waveTimeData; }

/**
 * @brief Updates the model time in the wave time data.
 * @param time The new model time.
 */
void updateWaveModelTime(const ww4_utils::DateTime &time) {
  waveTimeData.modelTime = time;
}

/**
 * @brief Updates the input time data for a specific input type.
 * @param type The input type to update.
 * @param data The new input time data.
 */
void updateWaveInputTime(ww4_utils::InputType type,
                         const ww4_utils::intTimeData &data) {
  switch (type) {
  case ww4_utils::InputType::WaterLevels:
    waveTimeData.waterLevels = data;
    break;
  case ww4_utils::InputType::Currents:
    waveTimeData.currents = data;
    break;
  case ww4_utils::InputType::Winds:
    waveTimeData.winds = data;
    break;
  case ww4_utils::InputType::IceConcentrations:
    waveTimeData.iceConcentrations = data;
    break;
  case ww4_utils::InputType::BottomDepth:
    waveTimeData.bottomDepth = data;
    break;
  }
}

// --- resetInternalState -----------------------------------------------------
/**
 * @brief Resets the internal state of the core module.
 * @details Clears global configuration, program name, and ensures log file
 *          stream is reset. Used during program finalization.
 */
void resetInternalState() noexcept {
  globalRunConfig = ww4_utils::RunConfig();
  capturedProgramName.clear();
  if (logFile.is_open()) {
    logFile.close();
  }
  waveTimeData = ww4_utils::waveTimeData();
  ww4_utils::resetInputData();
}

} // namespace ww4_core
