/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_std_out.cpp
 * @brief Implementation of standard screen output routines.
 * @details This file implements the routines for managing screen output,
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
 * @date Last update : 2026-10-27
 * @note Converted from WAVEWATCH III (ww3_shel.F90, ww3_multi.F90, and
 *       w3servmd.F90).
 *       Original author: Hendrik L. Tolman.
 */

#include "ww4_utils/ww4_std_out.h"
#include <cstdlib>
#include <iomanip>

namespace ww4_utils {

namespace ww4_std_out {

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

// --- writeExtcdeOutput ------------------------------------------------------
void writeExtcdeOutput(std::ostream &os, std::optional<std::string_view> msg,
                       std::optional<std::string_view> file,
                       std::optional<int> line) {
  const std::string prefix = "WW4 ERROR:";

  if (msg.has_value()) {
    os << prefix << " " << *msg << "\n";
  }

  std::string context;
  if (file.has_value()) {
    context += " FILE=";
    context += *file;
  }
  if (line.has_value()) {
    context += " LINE=";
    context += std::to_string(*line);
  }

  if (!context.empty()) {
    os << prefix << context << "\n";
  }
  os << std::flush;
}

// --- writeWarningOutput -----------------------------------------------------
void writeWarningOutput(std::ostream &os, std::string_view msg,
                        std::optional<std::string_view> file,
                        std::optional<int> line) {
  const std::string prefix = "WW4 WARNING:";

  os << prefix << " " << msg << "\n";

  std::string context;
  if (file.has_value()) {
    context += " FILE=";
    context += *file;
  }
  if (line.has_value()) {
    context += " LINE=";
    context += std::to_string(*line);
  }

  if (!context.empty()) {
    os << prefix << context << "\n";
  }
  os << std::flush;
}

// --- extcde -----------------------------------------------------------------
[[noreturn]] void extcde(int exitCode, std::ostream &os,
                         std::optional<std::string_view> msg,
                         std::optional<std::string_view> file,
                         std::optional<int> line) {
  if (&os == &std::cout || &os == &std::cerr) {
    writeExtcdeOutput(std::cerr, msg, file, line);
  } else {
    writeExtcdeOutput(os, msg, file, line);
    writeExtcdeOutput(std::cerr, msg, file, line);
  }
  std::exit(exitCode);
}

// --- warning ----------------------------------------------------------------
void warning(std::ostream &os, std::string_view msg,
             std::optional<std::string_view> file, std::optional<int> line) {
  writeWarningOutput(os, msg, file, line);
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

} // namespace ww4_std_out
} // namespace ww4_utils
