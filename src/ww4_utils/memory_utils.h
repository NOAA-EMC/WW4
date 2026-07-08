/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file memory_utils.h
 * @brief Utilities for capturing memory usage of the current process.
 * @details This header defines the MemoryUsage structure and MemoryUtils class,
 *          providing functionality to read process memory metrics from the
 * operating system.
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

#pragma once

#include <cstdint>
#include <optional>

namespace ww4_utils {

void setMemoryStatusPathForTesting(const char *path);

void resetMemoryStatusPath() noexcept;

/**
 * @struct MemoryUsage
 * @brief Represents various memory usage metrics of a process.
 * @details Values are typically in kilobytes (kB).
 * @var MemoryUsage::vmPeak
 * @brief Peak virtual memory size.
 * @var MemoryUsage::vmSize
 * @brief Virtual memory size.
 * @var MemoryUsage::vmHWM
 * @brief Peak resident set size ("High Water Mark").
 * @var MemoryUsage::vmRSS
 * @brief Resident set size.
 */
struct MemoryUsage {
  std::uint64_t vmPeak{0};
  std::uint64_t vmSize{0};
  std::uint64_t vmHWM{0};
  std::uint64_t vmRSS{0};
};

class MemoryUtils {
public:
  [[nodiscard]] static std::optional<MemoryUsage> captureMemoryUsage() noexcept;

  [[nodiscard]] static std::optional<std::uint64_t> captureMemoryHWM() noexcept;
};

} // namespace ww4_utils
