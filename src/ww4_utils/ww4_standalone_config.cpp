/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_standalone_config.cpp
 * @brief Implementation of stand-alone configuration processing.
 * @details This file implements the loadStandaloneConfig function,
 *          providing a simple YAML parser for start and end times.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date Initial, 2026-04-02
 * @date Last update : 2026-07-07
 */

#include "ww4_utils/ww4_standalone_config.h"
#include "ww4_utils/ww4_run_config.h"
#include <charconv>
#include <fstream>
#include <string>
#include <yaml-cpp/yaml.h>

/**
 * @namespace ww4_utils
 * @brief Utilities for WAVEWATCH IV.
 */
namespace ww4_utils {

/**
 * @brief Internal helper to parse a date-time string in "YYYYMMDD HHMMSS"
 * format.
 * @param s The string view to parse.
 * @return A DateTime structure if successful, or std::nullopt.
 */
std::optional<DateTime> parseDateTimeString(const std::string_view s) {
  const std::string_view clean = cleanValue(s);

  if (clean.length() < 15 || clean[8] != ' ')
    return std::nullopt;

  int ymd = 0;
  const auto [ptr1, ec1] = std::from_chars(clean.data(), clean.data() + 8, ymd);
  if (ec1 != std::errc())
    return std::nullopt;

  const std::string_view hms_part = clean.substr(9);
  double hms = 0.0;
  if (std::from_chars(hms_part.data(), hms_part.data() + hms_part.size(), hms)
          .ec != std::errc()) {
    return std::nullopt;
  }

  return DateTime{ymd, hms};
}

/**
 * @brief Loads the stand-alone configuration from a YAML file.
 * @details Reads the specified YAML file, extracts the start and end times
 *          from the 'simulation' node, and validates that the end time is
 *          not before the start time.
 * @param filename The name of the YAML file to load.
 * @param os Output stream for reporting.
 * @return A StandaloneConfig structure if successful, or std::nullopt
 *         if an error occurred (e.g., file not found, invalid format,
 *         or validation failure).
 * @author Main Author(s): Hendrik L. Tolman, Aldgisl (AI Persona)
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 */
std::optional<StandaloneConfig>
loadStandaloneConfig(const std::string_view filename,
                     std::ostream &os) noexcept {
  try {
    const YAML::Node config_node = YAML::LoadFile(std::string(filename));

    if (!config_node["simulation"]) {
      os << "WW4 ERROR: Mandatory 'simulation' section missing in '" << filename
         << "'." << std::endl;
      return std::nullopt;
    }

    const auto sim_node = config_node["simulation"];
    StandaloneConfig config{};

    bool startFound = false;
    if (sim_node["start_time"]) {
      const auto dt =
          parseDateTimeString(sim_node["start_time"].as<std::string>());
      if (dt) {
        config.startTime = *dt;
        startFound = true;
      } else {
        os << "WW4 ERROR: Invalid start_time format in '" << filename
           << "'. Expected format: \"YYYYMMDD HHMMSS\"" << std::endl;
      }
    }

    bool endFound = false;
    if (sim_node["end_time"]) {
      const auto dt =
          parseDateTimeString(sim_node["end_time"].as<std::string>());
      if (dt) {
        config.endTime = *dt;
        endFound = true;
      } else {
        os << "WW4 ERROR: Invalid end_time format in '" << filename
           << "'. Expected format: \"YYYYMMDD HHMMSS\"" << std::endl;
      }
    }

    if (!startFound || !endFound) {
      os << "WW4 ERROR: Mandatory field(s) missing in 'simulation' section of '"
         << filename << "':" << std::endl;
      if (!startFound)
        os << "           Missing: start_time" << std::endl;
      if (!endFound)
        os << "           Missing: end_time" << std::endl;
      return std::nullopt;
    }

    // Validation: endTime >= startTime
    if (TimeManagement::differenceInSeconds(config.startTime, config.endTime) <
        0.0) {
      os << "WW4 ERROR: End time before start time in '" << filename
         << "':" << std::endl;
      os << "           Start time: "
         << TimeManagement::toFormattedString(config.startTime) << std::endl;
      os << "           End time:   "
         << TimeManagement::toFormattedString(config.endTime) << std::endl;
      return std::nullopt;
    }

    return config;
  } catch (const YAML::Exception &e) {
    os << "WW4 ERROR: Failed to load/parse stand-alone configuration file '"
       << filename << "': " << e.what() << std::endl;
    return std::nullopt;
  }
}

/**
 * @brief Reports the stand-alone configuration to the provided output stream.
 * @param config The StandaloneConfig structure to report.
 * @param os The output stream to write to (default: std::cout).
 * @author Main Author(s): Hendrik L. Tolman, Aldgisl (AI Persona)
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date 2026-05-01
 */
void reportStandaloneConfig(const StandaloneConfig &config, std::ostream &os) {
  os << "\n  Stand-alone configuration settings :" << std::endl;
  os << "     Start time         : "
     << TimeManagement::toFormattedString(config.startTime) << std::endl;
  os << "     End time           : "
     << TimeManagement::toFormattedString(config.endTime) << std::endl;
  os << std::endl;
}

} // namespace ww4_utils
