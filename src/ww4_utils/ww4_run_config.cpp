/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file ww4_run_config.cpp
 * @brief Implementation of run-time configuration processing.
 * @details This file implements the loadRunConfig function,
 *          providing a simple YAML parser for run-time settings.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 * @date Initial, 2026-04-03
 * @date Last update : 2026-07-07
 */

#include "ww4_utils/ww4_run_config.h"
#include "ww4_utils/ww4_standalone_config.h"
#include "ww4_utils/ww4_std_out.h"
#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <string>
#include <yaml-cpp/yaml.h>

namespace ww4_utils {

namespace {

/**
 * @brief Helper to parse a homogeneous data string.
 * @param s The string to parse (format: "YYYYMMDD HHMMSS val1 val2 ...").
 * @return A HomogeneousDataPoint if successful.
 */
std::optional<HomogeneousDataPoint> parseHomogeneousString(std::string_view s) {
  if (s.empty())
    return std::nullopt;

  const std::string_view cleanS = cleanValue(s);
  const size_t firstSpace = cleanS.find(' ');
  if (firstSpace == std::string_view::npos)
    return std::nullopt;

  const size_t secondSpace = cleanS.find(' ', firstSpace + 1);
  if (secondSpace == std::string_view::npos)
    return std::nullopt;

  const std::string_view dateTimePart = cleanS.substr(0, secondSpace);
  const auto dt = parseDateTimeString(dateTimePart);
  if (!dt)
    return std::nullopt;

  HomogeneousDataPoint dp;
  dp.time = *dt;

  std::string_view remaining = cleanS.substr(secondSpace);
  while (!remaining.empty()) {
    const size_t firstNotSpace = remaining.find_first_not_of(' ');
    if (firstNotSpace == std::string_view::npos)
      break;
    remaining = remaining.substr(firstNotSpace);
    const size_t nextSpace = remaining.find(' ');
    const std::string_view valStr = remaining.substr(0, nextSpace);

    double val = 0.0;
    if (std::from_chars(valStr.data(), valStr.data() + valStr.size(), val).ec ==
        std::errc()) {
      dp.values.push_back(val);
    } else {
      return std::nullopt;
    }

    if (nextSpace == std::string_view::npos)
      break;
    remaining = remaining.substr(nextSpace);
  }

  if (dp.values.empty())
    return std::nullopt;

  return dp;
}

/**
 * @brief Helper to parse InputFieldOption from string.
 * @param value The string value to parse.
 * @param allowFromGrid Whether to allow the 'from_grid' option.
 * @return The corresponding InputFieldOption.
 */
InputFieldOption parseInputOption(const std::string_view value,
                                  const bool allowFromGrid = false) {
  if (value == "none") {
    return InputFieldOption::None;
  } else if (value == "from_file") {
    return InputFieldOption::FromFile;
  } else if (value == "from_coupling") {
    return InputFieldOption::FromCoupling;
  } else if (value == "homogeneous") {
    return InputFieldOption::Homogeneous;
  } else if (value == "from_grid") {
    if (allowFromGrid) {
      return InputFieldOption::FromGrid;
    }
  }
  return InputFieldOption::Undefined;
}

/**
 * @brief Helper to convert InputFieldOption to string for reporting.
 * @param option The InputFieldOption to convert.
 * @return A string representation of the option.
 */
std::string inputOptionToString(const InputFieldOption option) {
  switch (option) {
  case InputFieldOption::None:
    return "none";
  case InputFieldOption::FromFile:
    return "from_file";
  case InputFieldOption::FromCoupling:
    return "from_coupling";
  case InputFieldOption::Homogeneous:
    return "homogeneous";
  case InputFieldOption::FromGrid:
    return "from_grid";
  default:
    return "undefined";
  }
}

/**
 * @brief Helper to update OutputConfig from a YAML node.
 * @param oc The OutputConfig structure to update.
 * @param node The YAML node for the output type.
 */
void parseOutputConfig(OutputConfig &oc, const YAML::Node &node) {
  if (!node)
    return;

  if (node["requested"]) {
    oc.requested = (node["requested"].as<std::string>() == "yes");
  }
  if (node["interval"]) {
    oc.interval = node["interval"].as<double>();
  }
  if (node["start"]) {
    oc.startTime = parseDateTimeString(node["start"].as<std::string>());
  }
  if (node["end"]) {
    oc.endTime = parseDateTimeString(node["end"].as<std::string>());
  }
  if (node["at_first"]) {
    oc.atFirstTime = (node["at_first"].as<std::string>() == "yes");
  }
}

/**
 * @brief Helper to echo a homogeneous data series.
 * @param processed Vector of data points.
 * @param fieldName Name of the field.
 * @param option Echo level.
 * @param os Output stream.
 */
void echoHomogeneousData(const std::vector<HomogeneousDataPoint> &processed,
                         std::string_view /*fieldName*/, EchoOption option,
                         std::ostream &os) {
  if (processed.empty() || option == EchoOption::None)
    return;

  if (option == EchoOption::Summary) {
    os << "        Number of data points: " << processed.size() << std::endl;
  } else if (option == EchoOption::Full) {
    for (const auto &dp : processed) {
      os << "        " << TimeManagement::toFormattedString(dp.time) << " :";
      for (const auto val : dp.values) {
        os << " " << val;
      }
      os << std::endl;
    }
  }
}

/**
 * @brief Helper to report OutputConfig settings.
 * @param oc The OutputConfig structure to report.
 * @param label The label for the output type.
 * @param os The output stream to write to.
 */
void reportOutput(const OutputConfig &oc, const std::string_view label,
                  std::ostream &os) {
  if (oc.requested) {
    os << "\n     " << label << " output" << std::endl;
    if (label != "API") {
      os << "        Interval          : " << oc.interval << " s" << std::endl;
      os << "        At first time     : " << (oc.atFirstTime ? "yes" : "no")
         << std::endl;
    }
    if (oc.startTime) {
      os << "        Start time        : "
         << TimeManagement::toFormattedString(*oc.startTime) << std::endl;
    }
    if (oc.endTime) {
      os << "        End time          : "
         << TimeManagement::toFormattedString(*oc.endTime) << std::endl;
    }
  } else {
    os << "\n     " << label << " output not requested " << std::endl;
  }
}

} // namespace

/**
 * @brief Internal helper to trim whitespace and quotes from a string.
 * @param s The string view to clean.
 * @return A cleaned string view.
 */
std::string_view cleanValue(const std::string_view s) {
  const size_t start = s.find_first_not_of(" \t\r\n\"");
  const size_t end = s.find_last_not_of(" \t\r\n\"");
  if (start == std::string_view::npos)
    return "";
  return s.substr(start, end - start + 1);
}

/**
 * @brief Loads the run-time configuration from a YAML file.
 * @details Reads the specified YAML file, extracts configuration settings
 *          from nested sections (general, physics, forcing, homogeneous_data,
 *          output), and updates the TimeManagement calendar type.
 * @param filename The name of the YAML file to load.
 * @param os Output stream for reporting.
 * @return A RunConfig structure containing the loaded (or default) settings,
 *         or std::nullopt if the file could not be opened.
 * @author Main Author(s): Hendrik L. Tolman, Aldgisl (AI Persona)
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 */
std::optional<RunConfig> loadRunConfig(const std::string_view filename,
                                       std::ostream &os) noexcept {
  try {
    const YAML::Node config_node = YAML::LoadFile(std::string(filename));

    RunConfig config{};

    // 1. General section
    if (const auto node = config_node["general"]) {
      if (node["calendar_type"]) {
        const auto val = node["calendar_type"].as<std::string>();
        if (val == "Standard") {
          config.calendarType = TimeManagement::CalendarType::Standard;
        } else if (val == "NoLeap") {
          config.calendarType = TimeManagement::CalendarType::NoLeap;
        } else if (val == "ThreeSixtyDay") {
          config.calendarType = TimeManagement::CalendarType::ThreeSixtyDay;
        }
      }
      if (node["produce_std_out"]) {
        config.produceStdOut =
            (node["produce_std_out"].as<std::string>() == "yes");
      }
      if (node["produce_log_file"]) {
        config.produceLogFile =
            (node["produce_log_file"].as<std::string>() == "yes");
      }
      if (node["screen_output_level"]) {
        const auto val = node["screen_output_level"].as<std::string>();
        if (val == "none") {
          config.screenOutputLevel = ScreenOutputLevel::None;
        } else if (val == "summary") {
          config.screenOutputLevel = ScreenOutputLevel::Summary;
        } else if (val == "full") {
          config.screenOutputLevel = ScreenOutputLevel::Full;
        }
      }
      if (node["time_step"]) {
        config.timeStep = node["time_step"].as<double>();
      }
    }

    // 2. Physics section
    if (const auto node = config_node["physics"]) {
      if (node["dry_run"]) {
        config.dryRun = (node["dry_run"].as<std::string>() == "yes");
      }
      if (node["propagate_x"]) {
        config.propagateX = (node["propagate_x"].as<std::string>() == "yes");
      }
      if (node["propagate_y"]) {
        config.propagateY = (node["propagate_y"].as<std::string>() == "yes");
      }
      if (node["propagate_theta"]) {
        config.propagateTheta =
            (node["propagate_theta"].as<std::string>() == "yes");
      }
      if (node["propagate_k"]) {
        config.propagateK = (node["propagate_k"].as<std::string>() == "yes");
      }
      if (node["source_terms"]) {
        config.sourceTerms = (node["source_terms"].as<std::string>() == "yes");
      }
    }

    // 3. Forcing section
    if (const auto node = config_node["forcing"]) {
      if (node["bottom_depth"]) {
        config.bottomDepth =
            parseInputOption(node["bottom_depth"].as<std::string>(), true);
      }
      if (node["water_levels"]) {
        config.waterLevels =
            parseInputOption(node["water_levels"].as<std::string>());
      }
      if (node["currents"]) {
        config.currents = parseInputOption(node["currents"].as<std::string>());
      }
      if (node["winds"]) {
        config.winds = parseInputOption(node["winds"].as<std::string>());
      }
      if (node["ice_concentrations"]) {
        config.iceConcentrations =
            parseInputOption(node["ice_concentrations"].as<std::string>());
      }
      if (node["echo_hom_input"]) {
        const auto val = node["echo_hom_input"].as<std::string>();
        if (val == "none") {
          config.echoHomInput = EchoOption::None;
        } else if (val == "summary") {
          config.echoHomInput = EchoOption::Summary;
        } else if (val == "full") {
          config.echoHomInput = EchoOption::Full;
        }
      }
    }

    // 4. Homogeneous Data section
    if (const auto node = config_node["homogeneous_data"]) {
      auto parsePoints = [&](const std::string &key,
                             std::vector<HomogeneousDataPoint> &list) {
        if (node[key]) {
          for (const auto &item : node[key]) {
            auto dp = parseHomogeneousString(item.as<std::string>());
            if (dp) {
              list.push_back(*dp);
            }
          }
        }
      };
      parsePoints("water_levels", config.homogeneousWaterLevels);
      parsePoints("currents", config.homogeneousCurrents);
      parsePoints("winds", config.homogeneousWinds);
      parsePoints("ice_concentrations", config.homogeneousIceConcentrations);
      parsePoints("bottom_depth", config.homogeneousBottomDepth);
    }

    // 5. Output section
    if (const auto node = config_node["output"]) {
      parseOutputConfig(config.outputFields, node["fields"]);
      parseOutputConfig(config.outputPoints, node["points"]);
      parseOutputConfig(config.outputRestart, node["restart"]);
      parseOutputConfig(config.outputApi, node["api"]);
    }

    // Mandatory fields check
    if (config.waterLevels == InputFieldOption::Undefined ||
        config.currents == InputFieldOption::Undefined ||
        config.winds == InputFieldOption::Undefined ||
        config.iceConcentrations == InputFieldOption::Undefined ||
        config.bottomDepth == InputFieldOption::Undefined) {
      os << "WW4 ERROR: Mandatory model input field(s) missing or invalid "
            "in configuration."
         << std::endl;
      if (config.waterLevels == InputFieldOption::Undefined)
        os << "   Missing/invalid: forcing -> water_levels" << std::endl;
      if (config.currents == InputFieldOption::Undefined)
        os << "   Missing/invalid: forcing -> currents" << std::endl;
      if (config.winds == InputFieldOption::Undefined)
        os << "   Missing/invalid: forcing -> winds" << std::endl;
      if (config.iceConcentrations == InputFieldOption::Undefined)
        os << "   Missing/invalid: forcing -> ice_concentrations" << std::endl;
      if (config.bottomDepth == InputFieldOption::Undefined)
        os << "   Missing/invalid: forcing -> bottom_depth" << std::endl;

      ww4_std_out::extcde(1, os, "Missing or invalid mandatory fields.",
                          __FILE__, __LINE__);
    }

    // Time step validation
    if (config.timeStep < 0.0) {
      os << "WW4 ERROR: Mandatory time step missing or invalid "
            "in configuration."
         << std::endl;
      ww4_std_out::extcde(1, os, "Missing or invalid mandatory time step.",
                          __FILE__, __LINE__);
    }

    // Output validation
    bool outputValid = true;
    auto validateOutput = [&](const OutputConfig &oc,
                              const std::string_view name) {
      if (oc.requested && name != "api" && oc.interval <= 0.0) {
        os << "WW4 ERROR: Mandatory interval missing or invalid for "
              "requested output: "
           << name << std::endl;
        return false;
      }
      return true;
    };

    outputValid &= validateOutput(config.outputFields, "fields");
    outputValid &= validateOutput(config.outputApi, "api");
    outputValid &= validateOutput(config.outputPoints, "points");
    outputValid &= validateOutput(config.outputRestart, "restart");

    if (!outputValid) {
      ww4_std_out::extcde(1, os,
                          "Missing or invalid mandatory output interval(s).",
                          __FILE__, __LINE__);
    }

    // Update TimeManagement with the loaded calendar type.
    TimeManagement::setCalendarType(config.calendarType);

    return config;
  } catch (const YAML::Exception &e) {
    os << "WW4 ERROR: Failed to load/parse run configuration file '" << filename
       << "': " << e.what() << std::endl;
    return std::nullopt;
  }
}

/**
 * @brief Reports the current configuration to the provided output stream.
 * @param config The RunConfig structure to report.
 * @param os The output stream to write to (default: std::cout).
 * @author Main Author(s): Hendrik L. Tolman, Aldgisl (AI Persona)
 * @author Contributors: Jules (Agentic AI), Kit Stokes, Jessica Meixner
 */
void reportRunConfig(const RunConfig &config, std::ostream &os) {
  os << "\n  Configuration settings :" << std::endl;

  std::string calType = "Standard";
  if (config.calendarType == TimeManagement::CalendarType::NoLeap) {
    calType = "NoLeap";
  } else if (config.calendarType ==
             TimeManagement::CalendarType::ThreeSixtyDay) {
    calType = "ThreeSixtyDay";
  }

  os << "     Calendar type        : " << calType << std::endl;
  os << "     Screen output        : " << (config.produceStdOut ? "yes" : "no")
     << std::endl;
  os << "     Log file             : " << (config.produceLogFile ? "yes" : "no")
     << std::endl;

  std::string echoStr = "summary";
  if (config.echoHomInput == EchoOption::None) {
    echoStr = "none";
  } else if (config.echoHomInput == EchoOption::Full) {
    echoStr = "full";
  }
  os << "     Echo input           : " << echoStr << std::endl;

  std::string screenStr = "full";
  if (config.screenOutputLevel == ScreenOutputLevel::None) {
    screenStr = "none";
  } else if (config.screenOutputLevel == ScreenOutputLevel::Summary) {
    screenStr = "summary";
  }
  os << "     Screen output level  : " << screenStr << std::endl;

  const bool isConventional = !config.dryRun && config.propagateX &&
                              config.propagateY && config.propagateTheta &&
                              config.propagateK && config.sourceTerms;

  if (isConventional) {
    os << "     Conventional model run" << std::endl;
  } else {
    os << "     Unconventional model run" << std::endl;
    if (config.dryRun) {
      os << "        Dry run" << std::endl;
    } else {
      os << "        Propagate X       : " << (config.propagateX ? "yes" : "no")
         << std::endl;
      os << "        Propagate Y       : " << (config.propagateY ? "yes" : "no")
         << std::endl;
      os << "        Propagate Theta   : "
         << (config.propagateTheta ? "yes" : "no") << std::endl;
      os << "        Propagate K       : " << (config.propagateK ? "yes" : "no")
         << std::endl;
      os << "        Source terms      : "
         << (config.sourceTerms ? "yes" : "no") << std::endl;
    }
  }

  os << "     Time step            : " << config.timeStep << " s" << std::endl;

  os << "\n  Model input:" << std::endl;

  os << "     Bottom depth         : "
     << inputOptionToString(config.bottomDepth) << std::endl;
  if (config.bottomDepth == InputFieldOption::Homogeneous) {
    echoHomogeneousData(config.homogeneousBottomDepth, "bottom depth",
                        config.echoHomInput, os);
  }
  os << "     Water levels         : "
     << inputOptionToString(config.waterLevels) << std::endl;
  if (config.waterLevels == InputFieldOption::Homogeneous) {
    echoHomogeneousData(config.homogeneousWaterLevels, "water levels",
                        config.echoHomInput, os);
  }
  os << "     Currents             : " << inputOptionToString(config.currents)
     << std::endl;
  if (config.currents == InputFieldOption::Homogeneous) {
    echoHomogeneousData(config.homogeneousCurrents, "currents",
                        config.echoHomInput, os);
  }
  os << "     Winds                : " << inputOptionToString(config.winds)
     << std::endl;
  if (config.winds == InputFieldOption::Homogeneous) {
    echoHomogeneousData(config.homogeneousWinds, "winds", config.echoHomInput,
                        os);
  }
  os << "     Ice concentrations   : "
     << inputOptionToString(config.iceConcentrations) << std::endl;
  if (config.iceConcentrations == InputFieldOption::Homogeneous) {
    echoHomogeneousData(config.homogeneousIceConcentrations,
                        "ice concentrations", config.echoHomInput, os);
  }

  os << "\n  Model output:" << std::endl;

  reportOutput(config.outputFields, "Gridded fields", os);
  reportOutput(config.outputPoints, "Point", os);
  reportOutput(config.outputRestart, "Restart file", os);
  reportOutput(config.outputApi, "API", os);

  os << std::endl;
}

} // namespace ww4_utils
