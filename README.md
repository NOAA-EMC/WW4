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

### 1. Setting up the Compile Environment

Environment parameters needed for CMake are set externally by the user, depending on the target system hardware and software configuration.

#### On Linux (GCC, Clang, or Intel LLVM):
Set the C++ compiler via environment variables or CMake definitions:
```bash
# Using GCC
export CXX=g++
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Or specifying the compiler explicitly via CMake
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
```

#### On macOS (Apple Clang or Homebrew LLVM/GCC):
Ensure Xcode Command Line Tools are installed (`xcode-select --install`) or Homebrew compilers are available:
```bash
# Using default Apple Clang
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Using Homebrew GCC or LLVM
export CXX=/opt/homebrew/bin/g++-13
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

### 2. Building the Project

Standard development build (includes tests):
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### 3. NOAA Operational Builds (Disabling Testing)

For NOAA operational environments, testing dependencies and test executables can be completely disabled using the `-DWW4_ENABLE_TESTING=OFF` flag:
```bash
cmake -B build_ops -S . -DWW4_ENABLE_TESTING=OFF
cmake --build build_ops
```
When `WW4_ENABLE_TESTING=OFF`, GoogleTest dependencies and test targets are completely skipped during build configuration and execution.

### 4. Git Submodules for External Dependencies

WAVEWATCH IV includes required external dependencies as Git submodules in the `externals/` directory:
- `externals/yaml-cpp` (version 0.8.0)
- `externals/googletest` (version 1.14.0)

When cloning the repository, initialize submodules using:
```bash
git clone --recursive https://github.com/NOAA-EMC/WW4.git
```
Or if already cloned, initialize and update submodules using:
```bash
git submodule update --init --recursive
```

# 
<p align="right">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/noaa_logo.gif" alt="NOAA Logo" height="50" ; width="55">  
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/nws.jpg" alt="NWS Logo" height="50" width="50">  
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/ncep_logo.gif" alt="NCEP Logo" height="50" width="75"> 
 </p>
