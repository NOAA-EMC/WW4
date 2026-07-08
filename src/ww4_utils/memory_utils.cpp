/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file memory_utils.cpp
 * @brief Implementation of memory utility routines.
 * @details This file implements the methods defined in the MemoryUtils class,
 *          specifically for Linux systems using the /proc filesystem.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-02-27
 * @date Last update : 2026-07-07
 */

#include "ww4_utils/memory_utils.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <string>
#include <string_view>

/**
 * @namespace ww4_utils
 * @brief Utilities for WAVEWATCH IV.
 */
namespace ww4_utils {

namespace {
const char *defaultStatusFilePath = "/proc/self/status";
const char *statusFilePath = defaultStatusFilePath;
} // namespace

/**
 * @brief Sets the path to the process status file for testing purposes.
 * @param path The path to the mock status file.
 * @note This is for internal testing only and should not be used in production.
 */
void setMemoryStatusPathForTesting(const char *path) { statusFilePath = path; }

/**
 * @brief Resets the memory status path to the default "/proc/self/status".
 */
void resetMemoryStatusPath() noexcept {
  statusFilePath = defaultStatusFilePath;
}

/**
 * @class MemoryUtils
 * @brief Utility class for memory-related operations.
 * @details Provides static methods to query memory usage from the system.
 */

/**
 * @brief Captures the current memory usage of the calling process.
 * @details Reads metrics from /proc/self/status on Linux systems.
 * @return A MemoryUsage struct containing the captured metrics, or
 * std::nullopt if capture fails.
 * @pre The operating system must provide /proc/self/status (Linux).
 */
std::optional<MemoryUsage> MemoryUtils::captureMemoryUsage() noexcept {
  std::ifstream statusFile(statusFilePath);
  if (!statusFile.is_open()) {
    return std::nullopt;
  }

  MemoryUsage usage;
  std::string line;
  int fieldsFound = 0;

  while (std::getline(statusFile, line)) {
    std::string_view lineView(line);

    // Trim leading whitespace (though /proc/self/status usually doesn't have
    // it)
    const size_t firstNonSpace = lineView.find_first_not_of(" \t");
    if (firstNonSpace != std::string_view::npos) {
      lineView.remove_prefix(firstNonSpace);
    }

    const auto parseLine = [&](const std::string_view key,
                               std::uint64_t &member) {
      if (lineView.starts_with(key)) {
        const std::string_view valuePart = lineView.substr(key.size());
        const size_t firstDigit = valuePart.find_first_of("0123456789");
        if (firstDigit != std::string_view::npos) {
          const char *const first = valuePart.data() + firstDigit;
          const char *const last = valuePart.data() + valuePart.size();

          // Find the end of the digit sequence
          const char *const end = std::find_if_not(
              first, last, [](char c) { return std::isdigit(c); });

          if (std::from_chars(first, end, member).ec == std::errc()) {
            fieldsFound++;
          }
        }
      }
    };

    parseLine("VmPeak:", usage.vmPeak);
    parseLine("VmSize:", usage.vmSize);
    parseLine("VmHWM:", usage.vmHWM);
    parseLine("VmRSS:", usage.vmRSS);

    if (fieldsFound == 4) {
      break;
    }
  }

  if (fieldsFound == 0) {
    return std::nullopt;
  }

  return usage;
}

/**
 * @brief Captures the memory high water mark (HWM) of the calling process.
 * @details Reads the vmHWM metric from /proc/self/status on Linux systems.
 * @return The peak resident set size in kB, or std::nullopt if capture fails.
 * @pre The operating system must provide /proc/self/status (Linux).
 */
std::optional<std::uint64_t> MemoryUtils::captureMemoryHWM() noexcept {
  const auto usage = captureMemoryUsage();
  if (usage) {
    return usage->vmHWM;
  }
  return std::nullopt;
}

} // namespace ww4_utils
