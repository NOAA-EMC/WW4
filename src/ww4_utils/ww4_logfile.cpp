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
 * @date Last update : 2026-09-28
 * @note Converted from WAVEWATCH III (ww3_shel.F90 and ww3_multi.F90).
 *       Original author: Hendrik L. Tolman.
 */

#include "ww4_utils/ww4_logfile.h"
#include <iomanip>

namespace ww4_utils {

namespace ww4_logfile {

// --- anyAction --------------------------------------------------------------
bool LogTableData::anyAction() const {
  return wlUpdated || cuUpdated || wiUpdated || icUpdated || bdUpdated ||
         fieldsPerformed || pointsPerformed || restartPerformed || apiPerformed;
}

// --- reset ------------------------------------------------------------------
void LogTableData::reset() {
  wlUpdated = cuUpdated = wiUpdated = icUpdated = bdUpdated = false;
  fieldsPerformed = pointsPerformed = restartPerformed = apiPerformed = false;
}

// --- writeInitialOutput -----------------------------------------------------
void writeInitialOutput(std::ostream &os, std::string_view programName) {
  const std::string mid =
      "*** WAVEWATCH IV program " + std::string(programName) + " ***";

  const std::string bannerLine = "      " + mid + "      ";
  const std::string separator(bannerLine.length(), '=');

  os << "\n"
     << "               " << bannerLine << "\n"
     << "               " << separator << "\n"
     << std::endl;
}

// --- writeFinalOutput -------------------------------------------------------
void writeFinalOutput(std::ostream &os, std::string_view programName,
                      std::optional<double> initTime,
                      std::optional<double> elapsedTotal) {
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

// --- writeUpdatingField -----------------------------------------------------
void writeUpdatingField(std::ostream &os, std::string_view fieldName) {
  os << "    Updating " << fieldName << std::endl;
}

// --- writeInterpolationInfo -------------------------------------------------
void writeInterpolationInfo(std::ostream &os, const DateTime &time1,
                            const DateTime &time2) {
  os << "      Interpolation from " << TimeManagement::toFormattedString(time1)
     << " to " << TimeManagement::toFormattedString(time2) << std::endl;
}

// --- writeLogTableHeader ----------------------------------------------------
void writeLogTableHeader(std::ostream &os) {
  os << "\n  "
        "+-------------------------+---------------------+-----------------+\n"
     << "  |          Time           |       Inputs        |     Outputs     "
        "|\n"
     << "  |    (at end of step)     | WLV CUR WND ICE DPT | FLD PNT RST API "
        "|\n"
     << "  +-------------------------+---------------------+-----------------+"
     << std::endl;
}

// --- writeLogTableLine ------------------------------------------------------
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

// --- writeLogTableFooter ----------------------------------------------------
void writeLogTableFooter(std::ostream &os) {
  os << "  +-------------------------+---------------------+-----------------+"
     << std::endl;
}

} // namespace ww4_logfile
} // namespace ww4_utils
