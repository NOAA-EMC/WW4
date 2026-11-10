/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_standalone_args.cpp
 * @brief Implementation of stand-alone command line argument parsing.
 * @details This file implements the parseStandaloneArgs and
 * reportStandaloneArgs functions for retrieving start and end simulation times
 * from CLI args.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-10-22
 * @date Last update : 2026-10-22
 */

#include "ww4_progs/ww4_standalone_args.h"
#include <string>
#include <string_view>
#include <vector>

namespace ww4_progs {

// --- parseStandaloneArgs ---------------------------------------------------
std::optional<StandaloneArgs> parseStandaloneArgs(const int argc, char **argv,
                                                  std::ostream &os) {
  if (argc <= 1) {
    os << "WW4 ERROR: Missing command line arguments for stand-alone program."
       << std::endl;
    os << "Usage: ww4_standalone <start_date> <start_time> <end_date> "
          "<end_time>"
       << std::endl;
    os << "   or: ww4_standalone \"<start_date> <start_time>\" \"<end_date> "
          "<end_time>\""
       << std::endl;
    return std::nullopt;
  }

  // Check for help flags
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg(argv[i]);
    if (arg == "-h" || arg == "--help") {
      os << "Usage: ww4_standalone <start_date> <start_time> <end_date> "
            "<end_time>"
         << std::endl;
      os << "   or: ww4_standalone \"<start_date> <start_time>\" \"<end_date> "
            "<end_time>\""
         << std::endl;
      return std::nullopt;
    }
  }

  std::optional<ww4_utils::DateTime> startTime;
  std::optional<ww4_utils::DateTime> endTime;

  // Check for flag-based arguments (-s/--start, -e/--end)
  bool hasFlags = false;
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg(argv[i]);
    if (arg == "-s" || arg == "--start" || arg == "--start_time") {
      hasFlags = true;
      if (i + 2 < argc) {
        const std::string combined =
            std::string(argv[i + 1]) + " " + std::string(argv[i + 2]);
        auto dt = ww4_utils::parseDateTimeString(combined);
        if (dt) {
          startTime = dt;
          i += 2;
          continue;
        }
      }
      if (i + 1 < argc) {
        auto dt = ww4_utils::parseDateTimeString(argv[i + 1]);
        if (dt) {
          startTime = dt;
          i += 1;
          continue;
        }
      }
    } else if (arg == "-e" || arg == "--end" || arg == "--end_time") {
      hasFlags = true;
      if (i + 2 < argc) {
        const std::string combined =
            std::string(argv[i + 1]) + " " + std::string(argv[i + 2]);
        auto dt = ww4_utils::parseDateTimeString(combined);
        if (dt) {
          endTime = dt;
          i += 2;
          continue;
        }
      }
      if (i + 1 < argc) {
        auto dt = ww4_utils::parseDateTimeString(argv[i + 1]);
        if (dt) {
          endTime = dt;
          i += 1;
          continue;
        }
      }
    }
  }

  // Positional parsing if not parsed via flags
  if (!hasFlags) {
    if (argc == 3) {
      startTime = ww4_utils::parseDateTimeString(argv[1]);
      endTime = ww4_utils::parseDateTimeString(argv[2]);
    } else if (argc == 5) {
      const std::string startCombined =
          std::string(argv[1]) + " " + std::string(argv[2]);
      const std::string endCombined =
          std::string(argv[3]) + " " + std::string(argv[4]);
      startTime = ww4_utils::parseDateTimeString(startCombined);
      endTime = ww4_utils::parseDateTimeString(endCombined);
    }
  }

  if (!startTime || !endTime) {
    os << "WW4 ERROR: Invalid or missing date/time parameters." << std::endl;
    if (!startTime) {
      os << "           Missing/invalid start time." << std::endl;
    }
    if (!endTime) {
      os << "           Missing/invalid end time." << std::endl;
    }
    os << "Expected format: \"YYYYMMDD HHMMSS\" or YYYYMMDD HHMMSS"
       << std::endl;
    return std::nullopt;
  }

  if (ww4_utils::TimeManagement::differenceInSeconds(*startTime, *endTime) <
      0.0) {
    os << "WW4 ERROR: End time before start time in command line arguments:"
       << std::endl;
    os << "           Start time: "
       << ww4_utils::TimeManagement::toFormattedString(*startTime) << std::endl;
    os << "           End time:   "
       << ww4_utils::TimeManagement::toFormattedString(*endTime) << std::endl;
    return std::nullopt;
  }

  return StandaloneArgs{*startTime, *endTime};
}

// --- reportStandaloneArgs -------------------------------------------------
void reportStandaloneArgs(const StandaloneArgs &args, std::ostream &os) {
  os << "\n  Stand-alone configuration settings :" << std::endl;
  os << "     Start time         : "
     << ww4_utils::TimeManagement::toFormattedString(args.startTime)
     << std::endl;
  os << "     End time           : "
     << ww4_utils::TimeManagement::toFormattedString(args.endTime) << std::endl;
  os << std::endl;
}

} // namespace ww4_progs
