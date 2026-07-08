/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_logfile.cpp
 * @brief Implementation of log file output routines.
 * @details This file implements the routines for managing log file output,
 *          matching the formats from WAVEWATCH III, updated for WAVEWATCH IV.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-04-01
 * @date Last update : 2026-07-07
 * @note Converted from WAVEWATCH III (ww3_shel.F90 and ww3_multi.F90).
 *       Original author: Hendrik L. Tolman.
 */

#include "ww4_utils/ww4_logfile.h"
#include <iomanip>

/**
 * @namespace ww4_utils
 * @brief Utilities for WAVEWATCH IV.
 */
namespace ww4_utils {

/**
 * @namespace ww4_logfile
 * @brief Routines for log file output.
 */
namespace ww4_logfile {

/**
 * @brief Checks if any action (input update or output) occurred.
 * @return True if any flag is set.
 */
bool LogTableData::anyAction() const {
  return wlUpdated || cuUpdated || wiUpdated || icUpdated || bdUpdated ||
         fieldsPerformed || pointsPerformed || restartPerformed || apiPerformed;
}

/**
 * @brief Resets all flags to false.
 */
void LogTableData::reset() {
  wlUpdated = cuUpdated = wiUpdated = icUpdated = bdUpdated = false;
  fieldsPerformed = pointsPerformed = restartPerformed = apiPerformed = false;
}

/**
 * @brief Writes the initial log entry to the provided output stream.
 * @details Duplicates the initial log formatting from WW3.
 * @param os The output stream to write to (e.g., an std::ofstream).
 * @param programName The name of the executable program.
 */
void writeInitialOutput(std::ostream &os, std::string_view programName) {
  const std::string mid =
      "*** WAVEWATCH IV program " + std::string(programName) + " ***";

  // Generic banner format with 15X (15 spaces) and / (newline)
  const std::string bannerLine = "      " + mid + "      ";
  const std::string separator(bannerLine.length(), '=');

  os << "\n"
     << "               " << bannerLine << "\n"
     << "               " << separator << "\n"
     << std::endl;
}

/**
 * @brief Writes the final log entry to the provided output stream.
 * @details Duplicates the final log formatting from WW3.
 *          Optionally includes execution times and memory usage.
 * @param os The output stream to write to.
 * @param programName The name of the executable program.
 * @param initTime Optional initialization time in seconds.
 * @param elapsedTotal Optional total elapsed time in seconds.
 */
void writeFinalOutput(std::ostream &os, std::string_view programName,
                      std::optional<double> initTime,
                      std::optional<double> elapsedTotal) {
  // Matches FORMAT 997, 998, 999
  if (initTime.has_value()) {
    os << "\n  Initialization time :" << std::fixed << std::setprecision(2)
       << std::setw(10) << *initTime << " s\n";
  }
  if (elapsedTotal.has_value()) {
    os << "  Elapsed time " << std::fixed << std::setprecision(2)
       << std::setw(10) << *elapsedTotal << " s\n";
  }

  os << "\n  End of program \n"
     << " ==================================================\n"
     << "          WAVEWATCH IV program " << programName << " \n"
     << std::endl;
}

/**
 * @brief Writes a message identifying that an input field is being updated.
 * @param os The output stream to write to.
 * @param fieldName The name of the field being updated.
 */
void writeUpdatingField(std::ostream &os, std::string_view fieldName) {
  os << "    Updating " << fieldName << std::endl;
}

/**
 * @brief Writes interpolation interval information for an input field.
 * @param os The output stream to write to.
 * @param time1 First interpolation time tag.
 * @param time2 Second interpolation time tag.
 */
void writeInterpolationInfo(std::ostream &os, const DateTime &time1,
                            const DateTime &time2) {
  os << "      Interpolation from " << TimeManagement::toFormattedString(time1)
     << " to " << TimeManagement::toFormattedString(time2) << std::endl;
}

/**
 * @brief Writes the header of the tabular log output.
 * @param os The output stream to write to.
 */
void writeLogTableHeader(std::ostream &os) {
  os << "  "
        "+-------------------------+---------------------+-----------------+\n"
     << "  |          Time           |       Inputs        |     Outputs     "
        "|\n"
     << "  |    (at end of step)     | WLV CUR WND ICE DPT | FLD PNT RST API "
        "|\n"
     << "  +-------------------------+---------------------+-----------------+"
     << std::endl;
}

/**
 * @brief Adds a data line to the tabular log output.
 * @param os The output stream to write to.
 * @param time The time stamp for the end of the interval.
 * @param data The data flags for the line.
 */
void writeLogTableLine(std::ostream &os, const DateTime &time,
                       const LogTableData &data) {
  auto mark = [](bool b) { return b ? 'X' : ' '; };

  os << "  | " << TimeManagement::toFormattedString(time) << " |" << "  "
     << mark(data.wlUpdated) << "   " << mark(data.cuUpdated) << "   "
     << mark(data.wiUpdated) << "   " << mark(data.icUpdated) << "   "
     << mark(data.bdUpdated) << "  |" << "  " << mark(data.fieldsPerformed)
     << "   " << mark(data.pointsPerformed) << "   "
     << mark(data.restartPerformed) << "   " << mark(data.apiPerformed) << "  |"
     << std::endl;
}

/**
 * @brief Writes the footer of the tabular log output.
 * @param os The output stream to write to.
 */
void writeLogTableFooter(std::ostream &os) {
  os << "  +-------------------------+---------------------+-----------------+"
     << std::endl;
}

} // namespace ww4_logfile
} // namespace ww4_utils
