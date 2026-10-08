<p align="center">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/WW4_banner.jpg" alt="WW4 banner" height="100">
</p>

# WAVEWATCH IV (WW4) GitHub Workflows

This directory contains the GitHub Actions workflow configurations for the WAVEWATCH IV project. These workflows ensure code quality, documentation integrity, and system stability.


## Unit and Integration Tests

The testing workflow ensures that all Level 1 (unit) and Level 2 (integration) tests pass. For detailed information on running tests, please refer to the [Tests README](../../tests/README.md).

## Linting and Formatting

The linting workflow ensures that the code adheres to the project's coding standards and remains free of common programming errors.

### Running Linting Interactively

You can run the linting and formatting tools locally to verify your changes before committing them.

1.  **Code Formatting (Clang-Format):**
    To format your code according to the project style:
    ```bash
    find src tests -name "*.cpp" -o -name "*.h" | xargs clang-format -i
    ```

2.  **Static Analysis (Cppcheck):**
    To run static analysis on the codebase:
    ```bash
    cppcheck --enable=all --suppress=missingIncludeSystem --suppress=unmatchedSuppression --suppress=unusedFunction --suppress=missingInclude -I src src
    ```

### Running Linting Automatically

Linting is integrated into the project's Continuous Integration (CI) pipeline. It is executed automatically:

- On every push to any branch.
- For every pull request submitted to the repository.

The CI environment uses `clang-format` (in dry-run mode) and `cppcheck` to ensure code quality and consistency.

#
<p align="right">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/noaa_logo.gif" alt="NOAA Logo" height="50" width="55">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/nws.jpg" alt="NWS Logo" height="50" width="50">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/ncep_logo.gif" alt="NCEP Logo" height="50" width="75">
 </p>
