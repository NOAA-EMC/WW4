<p align="center">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/WW4_banner.jpg" alt="WW4 banner" style="height: 100px; "/>
</p>

# <p align="center"> WW4 tools directory </p>

Copies of tools created for WAVEWATCH IV (WW4) are gathered here in the `tools/` directory. These are the microtools to support repository management and testing. Macrotools to help develop WW4 applications, for instance to manipulate grids, are gathered in their own repositories.

The tools gathered here are documented on the [WW4 Tools](https://github.com/NOAA-EMC/WW4/wiki/Tools.md) page of the WW4 wiki.

# Compilation and Setup

For detailed compilation and setup instructions, please refer to the [Compilation and Environment Setup section in the main README](../README.md#compilation-and-environment-setup).

# Developer Tools

WAVEWATCH IV provides additional tools to support developers during the coding process.

## Test Availability Check

To check if unit tests are available for a specific file and its identified routines:

```bash
./tools/ww4_test_check.sh --help
./tools/ww4_test_check.sh --file <filename>
```
*Note: The filename should be provided without extension (e.g., `time_management`).*

Options:
- `--help`, `-h`: Show help message and exit.
- `--file <filename>`: The filename to check (without extension, e.g. `time_management`).

This tool will:
- Identify source files in `src/`.
- Isolate user-defined routines (functions and methods).
- Check the `tests/` directory for corresponding unit tests.
- Report the test coverage status for each identified routine.

## UQ Domain Plotting Tool

To plot or animate the domain maps stored in a NetCDF domain file:

```bash
./tools/plot_UQ_domains.sh --help
./tools/plot_UQ_domains.sh <domain_file.nc> [--grid <grid_file.nc>] [--type <format>] [--animate] [--step <n>] [--fps <rate>]
./tools/plot_UQ_domains.sh --input <domain_file.nc> [--grid <grid_file.nc>] [--type <format>] [--animate] [--step <n>] [--fps <rate>]
```

Options:
- `--help`, `-h`: Show help message and exit.
- `<domain_file.nc>`: Path to the NetCDF domain file (positional argument).
- `--input <domain_file.nc>`, `-i <domain_file.nc>`: Path to the NetCDF domain file (optional alternative to positional argument).
- `--grid <grid_file.nc>`, `-g <grid_file.nc>`: Path to the associated UGRID grid file containing `grid_mask` (optional, automatically searched if omitted).
- `--type <format>`, `-t <format>`: Output graphic file format/type (e.g. `png`, `jpg`, `jpeg`, `pdf`, `eps`, `gif`). Default: `png`. Output file is saved in the current working directory, replacing the input file's `.nc` extension with the graphic format extension.
- `--animate`: Generate an animated GIF showing the evolution across maps.
- `--step <n>`, `-n <n>`: Sample every n-th iteration / map in animations (default: `1`). Initial and final maps are always included.
- `--fps <rate>`: Frames per second for animation (default: `2`).

This tool:
- Reads the NetCDF domain output file.
- Displays the domain map on a Cartesian grid with equal aspect ratio.
- Colors adjacent domains with distinct colors using minimal chromatic graph coloring (up to 10 colors).
- Annotates base-0 domain numbers (`D0`, `D1`, ...) and domain size (active calculation points) in the center of each domain.
- Asks for graphic output type (e.g. `--type png`, `jpg`, `pdf`, `gif`) rather than a full file path.
- Always generates the output file in the current working directory from which the tool is executed, replacing the input `.nc` extension with the graphic extension.
- Generates single domain map image for final map iteration by default.
- Generates animated GIFs showing the evolution across all stored maps when `--type gif` or `--animate` is specified.

#
<p align="right">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/noaa_logo.gif" alt="NOAA Logo" style="height: 50px; width:55px;"/>
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/nws.jpg" alt="NWS Logo" style="height: 50px; width:50px;"/>
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/ncep_logo.gif" alt="NCEP Logo" style="height: 50px; width:75px;"/>
 </p>
