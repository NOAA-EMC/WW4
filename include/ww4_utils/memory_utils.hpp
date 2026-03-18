/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file memory_utils.hpp
 * @brief Utilities for capturing memory usage of the current process.
 * @details This header defines the MemoryUsage structure and MemoryUtils class,
 *          providing functionality to read process memory metrics from the
 * operating system.
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * @author Aldgisl, Hendrik L. Tolman (Initial, 2026-02-27)
 * @author Aldgisl, Hendrik L. Tolman (Last Update, 2026-03-18)
 * @date 2026-03-18
 */

#ifndef WW4_UTILS_MEMORY_UTILS_HPP
#define WW4_UTILS_MEMORY_UTILS_HPP

#include <optional>

/**
 * @namespace ww4_utils
 * @brief Utilities for WAVEWATCH IV.
 */
namespace ww4_utils {

/**
 * @struct MemoryUsage
 * @brief Represents various memory usage metrics of a process.
 * @details Values are typically in kilobytes (kB).
 */
struct MemoryUsage {
  long vmPeak{0}; ///< Peak virtual memory size.
  long vmSize{0}; ///< Virtual memory size.
  long vmHWM{0};  ///< Peak resident set size ("High Water Mark").
  long vmRSS{0};  ///< Resident set size.
};

/**
 * @class MemoryUtils
 * @brief Utility class for memory-related operations.
 * @details Provides static methods to query memory usage from the system.
 */
class MemoryUtils {
public:
  /**
   * @brief Captures the current memory usage of the calling process.
   * @details Reads metrics from /proc/self/status on Linux systems.
   * @return A MemoryUsage struct containing the captured metrics, or
   * std::nullopt if capture fails.
   * @pre The operating system must provide /proc/self/status (Linux).
   */
  [[nodiscard]] static std::optional<MemoryUsage> captureMemoryUsage() noexcept;
};

} // namespace ww4_utils

#endif // WW4_UTILS_MEMORY_UTILS_HPP
