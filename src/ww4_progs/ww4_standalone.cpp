/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_standalone.cpp
 * @brief Stand-alone wave model program.
 * @details This program provides a simplified stand-alone environment for
 *          running the WAVEWATCH IV wave model core.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-29
 * @note This program follows the structure of the stand-alone shell
 *       (ww3_shel.F90) in WAVEWATCH III.
 *       Original author of WW3 stand-alone shell: Hendrik L. Tolman.
 */

#include "ww4_core/w4core_finalize.h"
#include "ww4_core/w4core_init.h"
#include "ww4_core/w4core_wave.h"
#include "ww4_utils/time_management.h"
#include "ww4_utils/ww4_standalone_config.h"
#include "ww4_utils/ww4_std_out.h"
#include <exception>
#include <filesystem>
#include <iostream>

// --- main -------------------------------------------------------------------
/**
 * @brief Main entry point for the stand-alone WAVEWATCH IV executable.
 * @param argc Command line argument count.
 * @param argv Command line argument values.
 * @return Exit status code (0 for success, non-zero for failure).
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-03
 * @date Last update : 2026-09-29
 */
int main(int argc, char **argv) {
  try {
    // === Program initialization =============================================
    // --- Extract program name -----------------------------------------------
    std::string programName = "ww4_standalone";
    if (argc > 0) {
      programName = std::filesystem::path(argv[0]).stem().string();
    }

    // --- Load configuration from ww4_standalone.yaml file -------------------
    const auto config =
        ww4_utils::loadStandaloneConfig("ww4_standalone.yaml", std::cout);
    if (!config) {
      // Passing source file name (__FILE__) and line number (__LINE__) for
      // error reporting and location tracing
      ww4_utils::ww4_std_out::extcde(1, std::cerr,
                                     "Could not load stand-alone configuration",
                                     __FILE__, __LINE__);
    }

    ww4_utils::reportStandaloneConfig(*config, std::cout);

    // --- MPI initialization (if applicable) ---------------------------------

    // === Run initialization routine =========================================
    ww4_core::w4core_init(config->startTime, programName, std::cout);

    // === Run time stepping routine ==========================================
    ww4_core::w4core_wave(config->startTime, config->endTime, std::cout);

    // === Run finalization routine ===========================================
    ww4_core::w4core_finalize(config->endTime, std::cout);

    return 0;
  } catch (const std::exception &e) {
    ww4_utils::ww4_std_out::extcde(1, std::cerr, e.what(), __FILE__, __LINE__);
  } catch (...) {
    ww4_utils::ww4_std_out::extcde(1, std::cerr, "Unknown exception in main",
                                   __FILE__, __LINE__);
  }
}
