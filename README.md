<p align="center">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/WW4_banner.jpg" alt="WW4 banner" height="100">
</p>

# <p align="center"> Welcome to the WAVEWATCH IV<sup> TM</sup> (WW4<sup> TM</sup>) repository! </p>

Please visit the wiki pages that constitute the [WAVEWATCH IV portal ](https://github.com/NOAA-EMC/WW4/wiki) for more information on this software package. 

The documentation files (i.e. files with capitalized names and .md extensions) residing in the repository https://github.com/NOAA-EMC/WW4 are limited to this README.md file and required documentation defining the Intellectual Property Protection of WW4. The latter files are 
[INTENT.md](./INTENT.md) describing the intent of the IP protection, 
[LICENSE.md](./LICENSE.md) providing the licensing information for WW4, 
[CONTRIBUTORS.md](./CONTRIBUTORS.md) identifying those who have made significant contributions to WW4, 
and [TRADEMARK.md](./TRADEMARK.md) documenting the Trademark Status of WW4. 

The only other file in this format is [AGENTS.md](./AGENTS.md), which contains information defining an agentic AI approach used to create, translate or refactor code using AI agents such as Copilot or Jules, the latter of which has been used extensively in developing the WW4 code from WW3. Note that this AI agent is set up to automate coding standards, doxygen documentation and unit testing as mandated for WW4.

The only other file that could be construed as “documentation” in the home directory of the repository is the VERSION file.

## Compilation and Environment Setup

WAVEWATCH IV uses a standard CMake build system (v3.25+). All compilation is local to the active repository clone. The build process does not modify the user's interactive shell environment or profile scripts.

### 1. Cloning the Repository and Git Submodules

WAVEWATCH IV treats third-party dependencies (`yaml-cpp` and `googletest`) as Git submodules located within the `externals/` directory rather than requiring system-installed packages or external downloads:
- `externals/yaml-cpp` (version 0.8.0) - Configuration file parsing
- `externals/googletest` (version 1.14.0) - C++ unit testing framework (included when `WW4_ENABLE_TESTING=ON`)

CMake integrates these submodules directly into the build system using `add_subdirectory`, building them in-tree from source. This ensures self-contained, reproducible, and offline-friendly builds without external system library requirements.

When cloning the repository, include submodules using:
```bash
git clone --recursive https://github.com/NOAA-EMC/WW4.git
```

If the repository was already cloned without `--recursive`, initialize and update the submodules using:
```bash
git submodule update --init --recursive
```

### 2. Setting up the Compile Environment

You can specify the C++ compiler either by exporting the `CXX` environment variable or by passing `-DCMAKE_CXX_COMPILER` directly to CMake, depending on the target system hardware and software configuration.

#### On Linux (GCC, Clang, or Intel LLVM):
```bash
# Using GCC via environment variable
export CXX=g++
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Using Clang explicitly via CMake argument
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release

# Using Intel LLVM (icpx)
export CXX=icpx
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

#### On macOS (Apple Clang or Homebrew LLVM/GCC):
Ensure Xcode Command Line Tools are installed (`xcode-select --install`) or Homebrew compilers are available:
```bash
# Using default Apple Clang
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Using Homebrew GCC or LLVM
export CXX=/opt/homebrew/bin/g++-13
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Or specifying Homebrew LLVM explicitly via CMake argument
cmake -B build -S . -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ -DCMAKE_BUILD_TYPE=Release
```

### 3. Compiler and Build Options

WW4 supports standard CMake build configuration flags:

- **Build Type (`-DCMAKE_BUILD_TYPE`):** Set to `Release`, `Debug`, `RelWithDebInfo`, or `MinSizeRel`. Default is `Release`.
  ```bash
  cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
  ```
- **Strict Compiler Warnings (`-DWW4_STRICT_WARNINGS=ON|OFF`):** Enables strict compiler warnings (`-Wall -Wextra -Wpedantic -Werror` / `/W4 /WX`) with appropriate suppressions for external dependencies on supported compilers. Default is `OFF`.
  ```bash
  cmake -B build -S . -DWW4_STRICT_WARNINGS=ON
  ```
- **Sanitizers (`-DWW4_USE_SANITIZERS=ON|OFF`):** Enables AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan). Default is `OFF`.
  ```bash
  cmake -B build -S . -DWW4_USE_SANITIZERS=ON
  ```
- **Enable/Disable Testing (`-DWW4_ENABLE_TESTING=ON|OFF`):** Controls whether GoogleTest submodules and test targets are included in the build system. Default is `ON`.
  ```bash
  # For operational builds without testing components:
  cmake -B build_ops -S . -DWW4_ENABLE_TESTING=OFF
  cmake --build build_ops
  ```

### 4. Building the Project

Standard development build (includes tests):
```bash
# Configure build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build executables and libraries
cmake --build build
```

### 5. Operational Builds (Disabling Testing)

For operational environments, testing dependencies and test executables can be completely disabled using the `-DWW4_ENABLE_TESTING=OFF` flag:
```bash
cmake -B build_ops -S . -DWW4_ENABLE_TESTING=OFF
cmake --build build_ops
```
When `WW4_ENABLE_TESTING=OFF`, GoogleTest dependencies and test targets are completely skipped during build configuration and execution.

### 6. Cleaning Build Artifacts

To remove compiled object files, static libraries, and executables (including unit test executables located in `build/bin/`) generated during a build:
```bash
# Clean built object files, libraries, and executables target-by-target
cmake --build build --target clean
```

To perform a complete clean and remove all generated build directories and executable output folders:
```bash
# Remove build tree directory completely
rm -rf build

# Remove files in standalone executable directory if present
rm -rf exe/*
```

# 
<p align="right">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/noaa_logo.gif" alt="NOAA Logo" height="50" ; width="55">  
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/nws.jpg" alt="NWS Logo" height="50" width="50">  
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/ncep_logo.gif" alt="NCEP Logo" height="50" width="75"> 
 </p>
