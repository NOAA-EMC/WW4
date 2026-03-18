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
 * @author Aldgisl, Hendrik L. Tolman (Initial, 2026-02-27)
 * @author Aldgisl, Hendrik L. Tolman (Last Update, 2026-03-18)
 * @date 2026-03-18
 */

#include "ww4_utils/memory_utils.hpp"

#include <charconv>
#include <fstream>
#include <string>
#include <string_view>

namespace ww4_utils {

std::optional<MemoryUsage> MemoryUtils::captureMemoryUsage() noexcept {
  std::ifstream statusFile("/proc/self/status");
  if (!statusFile.is_open()) {
    return std::nullopt;
  }

  MemoryUsage usage;
  std::string line;
  int fieldsFound = 0;

  while (std::getline(statusFile, line)) {
    const std::string_view lineView(line);

    const auto parseLine = [&](const std::string_view key, long &member) {
      if (lineView.starts_with(key)) {
        const size_t pos = lineView.find_first_of("0123456789");
        if (pos != std::string_view::npos) {
          const char *const first = lineView.data() + pos;
          const char *const last = lineView.data() + lineView.size();
          if (std::from_chars(first, last, member).ec == std::errc()) {
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

} // namespace ww4_utils
