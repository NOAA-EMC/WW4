/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_grid_inspect.cpp
 * @brief Tool to inspect the contents of a WW4 NetCDF grid file.
 * @details Reads a NetCDF-UGRID grid file, captures errors, and prints
 *          grid metadata directly to standard output.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-11-04
 * @date Last update : 2026-11-04
 */

#include "ww4_core/solver_UQ/utils_UQ_IO/UQ_read_grid.h"
#include "ww4_utils/ww4_std_out.h"
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

// --- main -------------------------------------------------------------------
/**
 * @brief Main entry point for the WW4 NetCDF grid inspection tool.
 * @param argc Command line argument count.
 * @param argv Command line argument values.
 * @return Exit status code (0 for success, non-zero for failure).
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-11-04
 * @date Last update : 2026-11-04
 */
int main(int argc, char **argv) {
  try {
    // === Program initialization =============================================
    // --- Extract program name -----------------------------------------------
    std::string programName = "ww4_grid_inspect";
    if (argc > 0) {
      programName = std::filesystem::path(argv[0]).stem().string();
    }

    // --- Starting output ----------------------------------------------------
    ww4_utils::ww4_std_out::writeInitialOutput(std::cout, programName);

    // --- Validate command line arguments ------------------------------------
    if (argc < 2) {
      std::cout << "Usage: " << programName << " <grid_file.nc>\n" << std::endl;
      // Explanatory comment preceding first use of __FILE__ and __LINE__
      // Pass source location for error tracing
      ww4_utils::ww4_std_out::extcde(1, std::cout,
                                     "Missing grid file command line argument",
                                     __FILE__, __LINE__);
    }

    const std::string gridFile = argv[1];

    // === Spatial grid ingestion =============================================
    std::cout << "  Processing the grid:\n"
              << "     Grid file                : " << gridFile << std::endl;

    // Create UGRID data structure for reading grid geometry and metadata
    ww4::io::UgridGrid ugrid;

    // Attempt to read the file (all data)
    int status = ww4::io::readUQgrid(gridFile, ugrid);

    // Error processing
    if (status != 0) {
      std::string errMsg;
      if (status == 1) {
        errMsg = "Grid file does not exist or failed opening: " + gridFile;
      } else if (status == 2) {
        errMsg = "Failed reading metadata dimensions from file: " + gridFile;
      } else {
        errMsg = "Failed reading grid dataset arrays from file: " + gridFile;
      }
      // Failure to read file, abort program
      ww4_utils::ww4_std_out::extcde(status, std::cout, errMsg, __FILE__,
                                     __LINE__);
    }

    // Output of metadata
    ww4::io::printGridMetadata(ugrid, std::cout);

    // === Program finalization ===============================================
    // --- Ending output ------------------------------------------------------
    ww4_utils::ww4_std_out::writeFinalOutput(std::cout, programName);

    return 0;
  } catch (const std::exception &e) {
    ww4_utils::ww4_std_out::extcde(1, std::cerr, e.what(), __FILE__, __LINE__);
  } catch (...) {
    ww4_utils::ww4_std_out::extcde(1, std::cerr, "Unknown exception in main",
                                   __FILE__, __LINE__);
  }
}
