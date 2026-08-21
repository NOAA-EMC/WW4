<p align="center">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/WW4_banner.jpg" alt="WW4 banner" height="100">
</p>

# <p align="center"> WAVEWATCH IV<sup> TM</sup> (WW4<sup> TM</sup>) Configuration Templates </p>

This directory contains template files for configuring WAVEWATCH IV runtime environments. These templates provide a starting point for users to set up simulation parameters.

## Usage

Users can copy templates from this directory to the desired working directory (typically the repository root or execution directory) and modify them as needed:

1. Copy `templates/ww4_standalone.yaml` to configure simulation start/end times:
   ```bash
   cp templates/ww4_standalone.yaml ./ww4_standalone.yaml
   ```
2. Copy `templates/ww4_run_config.yaml` to configure general, physics, forcing, and output settings:
   ```bash
   cp templates/ww4_run_config.yaml ./ww4_run_config.yaml
   ```

Note: Runtime YAML configuration files created in the repository root are ignored by Git.

#
<p align="right">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/noaa_logo.gif" alt="NOAA Logo" height="50" ; width="55">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/nws.jpg" alt="NWS Logo" height="50" width="50">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/ncep_logo.gif" alt="NCEP Logo" height="50" width="75">
 </p>
