/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file w4core_finalize.cpp
 * @brief Implementation of the WAVEWATCH IV finalization routine.
 * @details This file implements the w4core_finalize routine.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-10-01
 * @note The architectural design of this routine follows the structure of
 *       the multi-grid shell (ww3_multi.F90) in WAVEWATCH III.
 *       Original author of WW3 multi-grid shell: Hendrik L. Tolman.
 */

#include "ww4_core/w4core_finalize.h"
#include "ww4_core/w4core_init.h"
#include "ww4_utils/memory_utils.h"
#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_input_utils.h"
#include "ww4_utils/ww4_logfile.h"
#include "ww4_utils/ww4_std_out.h"
#include <exception>
#include <iostream>

namespace ww4_core {

// --- w4core_finalize --------------------------------------------------------
/**
 * @brief Finalization routine for the WAVEWATCH IV core.
 * @details Performs all necessary cleanup and final reporting for the
 *          wave model core. Follows the architectural design of the
 *          finalization in ww3_multi.F90 from WAVEWATCH III.
 * @param endTime Simulation end time.
 * @param os Output stream for reporting.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-10-01
 */
void w4core_finalize(const ww4_utils::DateTime &endTime, std::ostream &os) {
  try {
    // === Capture run time ===================================================
    const double runTime = ww4_utils::TimeManagement::getProfilingTime();

    // === Final standard output ==============================================
    if (getRunConfig().produceStdOut) {
      os << "\n  Finalization (w4core_finalize) starting: "
         << ww4_utils::TimeManagement::toFormattedString(endTime) << "\n"
         << std::endl;

      os << "  Run ends at "
         << ww4_utils::TimeManagement::toFormattedString(
                ww4_utils::TimeManagement::getPresentDateTime())
         << std::endl;

      ww4_utils::ww4_std_out::writeFinalOutput(os, getProgramName(),
                                               std::nullopt, runTime);
    }

    // === Final log file output ==============================================
    if (getRunConfig().produceLogFile && getLogFileStream().is_open()) {
      ww4_utils::ww4_logfile::writeLogTableFooter(getLogFileStream());

      getLogFileStream() << "\n  Finalization (w4core_finalize) starting: "
                         << ww4_utils::TimeManagement::toFormattedString(
                                endTime)
                         << "\n"
                         << std::endl;

      getLogFileStream() << "  Run ends at "
                         << ww4_utils::TimeManagement::toFormattedString(
                                ww4_utils::TimeManagement::getPresentDateTime())
                         << std::endl;

      ww4_utils::ww4_logfile::writeFinalOutput(
          getLogFileStream(), getProgramName(), std::nullopt, runTime);

      getLogFileStream().close();
    }

    // === Release persistent model data =====================================
    if (getWaveTimeData().modelTime.has_value() &&
        *getWaveTimeData().modelTime != endTime) {
      // Explanatory comment preceding __FILE__ and __LINE__
      // Issue warning when model time mismatch occurs
      ww4_utils::ww4_std_out::warning(os, "Model time does not match end time.",
                                      __FILE__, __LINE__);
    }

    ww4_utils::TimeManagement::reset();
    ww4_utils::resetMemoryStatusPath();
    ww4_utils::resetInputData();
    resetInternalState();
  } catch (const std::exception &e) {
    // Explanatory comment preceding __FILE__ and __LINE__
    // Terminate execution on standard exception
    ww4_utils::ww4_std_out::extcde(1, os, e.what(), __FILE__, __LINE__);
  } catch (...) {
    // Explanatory comment preceding __FILE__ and __LINE__
    // Terminate execution on unknown exception
    ww4_utils::ww4_std_out::extcde(
        1, os, "Unknown exception in w4core_finalize", __FILE__, __LINE__);
  }
}

} // namespace ww4_core
