<p align="center">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/WW4_banner.jpg" alt="WW4 banner" height="100">
</p>

# External libraries used in WW4

External libraries used by WW4 are included as Git submodules in the `./externals` directory:
- `externals/yaml-cpp` (version 0.8.0) - Configuration file parsing
- `externals/googletest` (version 1.14.0) - C++ unit testing framework (when testing is enabled)

To clone WW4 with external dependencies included, use:
```bash
git clone --recursive https://github.com/NOAA-EMC/WW4.git
```

If the repository was cloned without `--recursive`, initialize and update submodules using:
```bash
git submodule update --init --recursive
```

#
<p align="right">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/noaa_logo.gif" alt="NOAA Logo" height="50" width="55">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/nws.jpg" alt="NWS Logo" height="50" width="50">
  <img src="https://github.com/NOAA-EMC/WW4/wiki/images/ncep_logo.gif" alt="NCEP Logo" height="50" width="75">
 </p>
