/**
 *       +--------------------------------------------------------+
 *       | WAVEWATCH IV, open source, code management by NOAA/NWS |
 *       +--------------------------------------------------------+
 *
 * @file UQ_nc_handle.h
 * @brief RAII wrapper for NetCDF file handles in WAVEWATCH IV UQ I/O.
 * @details Ensures exception-safe closing of NetCDF dataset handles (ncid).
 *
 * @copyright © 2026 National Weather Service, National Oceanic and Atmospheric
 * Administration. WAVEWATCH IV (TM) and WW4 (TM) are trademarks of the National
 * Weather Service.
 * NWS often uses Generative AI (GenAI) for code development and refactoring.
 * Whenever GenAI is used, NWS requires a full human review of code before it is
 * added to its repositories.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-11-09
 * @date Last update : 2026-11-09
 */

#pragma once

#include <netcdf.h>

namespace ww4::io {

// --- NcHandle ---------------------------------------------------------------
/**
 * @class NcHandle
 * @brief RAII wrapper for NetCDF file handle `ncid`.
 * @details Automatically closes the opened NetCDF file handle via `nc_close`
 * upon destruction or reassignment unless explicitly released.
 * @author Main Author(s): Aldgisl (AI Persona), Hendrik L. Tolman
 * @author Contributors: Jules (Agentic AI)
 * @date Initial, 2026-11-09
 * @date Last update : 2026-11-09
 */
class NcHandle {
public:
  explicit NcHandle(const int id = -1) noexcept : ncid_(id) {}

  ~NcHandle() {
    if (ncid_ >= 0) {
      nc_close(ncid_);
    }
  }

  NcHandle(const NcHandle &) = delete;
  NcHandle &operator=(const NcHandle &) = delete;

  NcHandle(NcHandle &&other) noexcept : ncid_(other.ncid_) { other.ncid_ = -1; }

  NcHandle &operator=(NcHandle &&other) noexcept {
    if (this != &other) {
      if (ncid_ >= 0) {
        nc_close(ncid_);
      }
      ncid_ = other.ncid_;
      other.ncid_ = -1;
    }
    return *this;
  }

  [[nodiscard]] int get() const noexcept { return ncid_; }

  int release() noexcept {
    const int id = ncid_;
    ncid_ = -1;
    return id;
  }

  void reset(const int new_id = -1) noexcept {
    if (ncid_ >= 0) {
      nc_close(ncid_);
    }
    ncid_ = new_id;
  }

private:
  int ncid_{-1};
};

} // namespace ww4::io
