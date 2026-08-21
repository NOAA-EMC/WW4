<p align="center">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/WW4_banner.jpg" alt="WW4 banner" style="height: 100px; "/>
</p>

# <p align="center"> WW4 tools directory </p>

Copies of tools created for WAVEWATCH IV (WW4) are gathered here in the `tools/` directory. These are the microtools to support repository management and testing. Macrotools to help develop WW4 applications, for instance to manipulate grids, are gathered in their own repositories.

The tools gathered here are documented on the [WW4 Tools](https://github.com/NOAA-EMC/WW4/wiki/Tools.md) page of the WW4 wiki.

# Compilation and Setup

WAVEWATCH IV uses a standard CMake build system. Build configuration is handled locally in each repository clone using CMake options and standard environment variables. The build system does not modify the user's interactive environment or shell profile scripts.

## Building WAVEWATCH IV

```bash
# Configure build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build executables and libraries
cmake --build build
```

For NOAA operational builds without testing components:
```bash
cmake -B build_ops -S . -DWW4_ENABLE_TESTING=OFF
cmake --build build_ops
```

# Developer Tools

WAVEWATCH IV provides additional tools to support developers during the coding process.

## Test Availability Check

To check if unit tests are available for a specific file and its identified routines:

```bash
./tools/ww4_test_check --file <filename>
```
*Note: The filename should be provided without extension (e.g., `time_management`).*

This tool will:
- Identify source files in `src/` and `include/`.
- Isolate user-defined routines (functions and methods).
- Check the `tests/` directory for corresponding unit tests.
- Report the test coverage status for each identified routine.

#
<p align="right">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/noaa_logo.gif" alt="NOAA Logo" style="height: 50px; width:55px;"/>
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/nws.jpg" alt="NWS Logo" style="height: 50px; width:50px;"/>
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/ncep_logo.gif" alt="NCEP Logo" style="height: 50px; width:75px;"/>
 </p>
