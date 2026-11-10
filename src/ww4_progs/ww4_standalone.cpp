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
 * @date Last update : 2026-10-22
 * @note This program follows the structure of the stand-alone shell
 *       (ww3_shel.F90) in WAVEWATCH III.
 *       Original author of WW3 stand-alone shell: Hendrik L. Tolman.
 */

#include "ww4_core/w4core_finalize.h"
#include "ww4_core/w4core_init.h"
#include "ww4_core/w4core_wave.h"
#include "ww4_progs/ww4_standalone_args.h"
#include "ww4_utils/time_management.h"
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
 * @date Last update : 2026-10-22
 */
int main(int argc, char **argv) {
  try {
    // === Program initialization =============================================
    // --- Extract program name -----------------------------------------------
    std::string programName = "ww4_standalone";
    if (argc > 0) {
      programName = std::filesystem::path(argv[0]).stem().string();
    }

    // --- Parse simulation start and end times from command line args ---------
    // Location tracing for error reporting
    const auto args = ww4_progs::parseStandaloneArgs(argc, argv, std::cout);
    if (!args) {
      ww4_utils::ww4_std_out::extcde(
          1, std::cerr, "Invalid command line arguments", __FILE__, __LINE__);
    }

    ww4_progs::reportStandaloneArgs(*args, std::cout);

    // --- MPI initialization (if applicable) ---------------------------------

    // === Run initialization routine =========================================
    ww4_core::w4core_init(args->startTime, programName, std::cout);

    // === Run time stepping routine ==========================================
    ww4_core::w4core_wave(args->startTime, args->endTime, std::cout);

    // === Run finalization routine ===========================================
    ww4_core::w4core_finalize(args->endTime, std::cout);

    return 0;
  } catch (const std::exception &e) {
    ww4_utils::ww4_std_out::extcde(1, std::cerr, e.what(), __FILE__, __LINE__);
  } catch (...) {
    ww4_utils::ww4_std_out::extcde(1, std::cerr, "Unknown exception in main",
                                   __FILE__, __LINE__);
  }
}
