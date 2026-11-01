# RONGO Board Library

This directory is reserved for verified hardware profiles used by RONGO. A profile describes a specific MCU or module, development board, and any relevant expansion board; it is not a generic description inferred from a chip family.

## Profile requirements

Each profile must identify the exact hardware model and revision, distinguish the MCU from the module and complete board, and record only verified interfaces, GPIO restrictions, and approved pin assignments. Include authoritative source references, such as manufacturer datasheets, board schematics, and other relevant documentation. Clearly mark unknown or unverified properties.

Profiles must have an explicit review or approval status. Do not treat a draft, an unreviewed report, or automatically generated configuration as an approved profile.

## WEP reports

Original WEP (Wepwawet ESP32 Profiler) JSON reports belong in `hardware/reports/`. They document observations from a particular hardware instance and are evidence for review, not approved board profiles. Compare report findings with manufacturer documentation before using them to prepare or update a profile.

Do not create a profile by copying a WEP report or importing its suggested PlatformIO configuration without review and approval.

No hardware profiles are currently included in this library.
