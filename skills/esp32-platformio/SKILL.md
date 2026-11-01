---
name: esp32-platformio
description: Use for ESP32 and PlatformIO tasks in RONGO: hardware identification, WEP reports, GPIO, build environments, dependencies, firmware uploads, and LittleFS.
---

# ESP32 and PlatformIO

## 1. Purpose and scope

Use this skill when analyzing or changing ESP32 and PlatformIO configuration in RONGO: selecting build environments, checking hardware compatibility, assigning GPIO, managing dependencies, building or uploading firmware, and managing the LittleFS filesystem.

Use it together with the project-wide rules in the root `AGENTS.md`. This skill defines the hardware workflow; it does not replace the general project requirements.

RONGO principle: where possible, keep one common codebase for compatible boards and isolate board-specific settings in PlatformIO environments. Do not assume that planned functionality has already been implemented.

## 2. Hardware identification

Distinguish between four levels:

1. **MCU** — the microcontroller, for example ESP32-S3.
2. **Module** — a module containing the MCU, RF circuitry, and memory, for example a specific ESP32-WROOM model.
3. **Development board** — a board carrying the module, USB interface, regulator, and pin routing.
4. **Expansion board or shield** — an additional board with connectors, circuitry, or attached peripherals.

Establish exact models and revisions before drawing compatibility conclusions, where possible. Record known information about flash, PSRAM, interfaces, available pins, and hardware restrictions separately.

Identifying the MCU is not sufficient to identify the complete development board. If the exact board or connected peripherals are unknown, say so; do not fill gaps with assumptions.

## 3. Using WEP reports

Look for original WEP reports in `hardware/reports/`, including nested directories. If the directory or reports do not exist, report that and ask for the available inspection results; do not fabricate a report.

For each JSON report:

1. Read the original file without changing or transforming it during analysis.
2. Identify the hardware instance and inspection it refers to, when the report provides that information.
3. Separate explicitly recorded data, conclusions derived from it, and unverified assumptions.
4. Compare findings with manufacturer documentation for the specific MCU, module, development board, and expansion board, where available.
5. Note disagreements, incomplete data, and properties that WEP cannot establish.
6. Present proposed PlatformIO changes separately and wait for approval before applying them.

WEP supports hardware discovery; it does not replace manufacturer documentation or confirm characteristics of the complete board. Do not turn a report or automatically generated settings into a board-library profile. Never automatically replace the main `platformio.ini` with them.

## 4. Working with PlatformIO

Inspect the actual `platformio.ini` structure, environment names, selected framework, platform, board identifiers, flash partitions, and dependencies before proposing changes.

For multiple compatible boards, prefer one common codebase and explicitly named `[env:...]` sections in the PlatformIO configuration. Isolate board differences in environment configuration, avoid firmware copies unless necessary, and do not duplicate settings without a technical reason.

Confirm board identifiers and dependency versions using the project configuration, installed board definitions, or authoritative documentation. Do not invent identifiers, compatibility, or versions.

Before changing configuration:

- Explain which boards and environments are affected.
- State which settings and dependencies will change and why.
- Check whether existing environments retain their behavior.
- Do not activate or replace an environment without approval.
- Do not automatically overwrite the main `platformio.ini` with a WEP-generated file.

## 5. Checking and assigning GPIO

Before proposing a pin, check not only the MCU GPIO but also the pinout of the specific board and schematics for connected modules and expansion boards. Use manufacturer documentation and an approved board profile, if available.

Check restrictions of the specific family and board: pins used by flash/PSRAM, boot and USB, strapping pins, input-only GPIO, ADC availability, peripheral functions, and possible conflicts with already connected devices. Do not assume a pin is free merely because its number appears in the MCU GPIO list.

For every proposed assignment, explain:

- The GPIO and interface to be used.
- The intended connected device.
- The evidence supporting the choice.
- Restrictions and potential conflicts.
- Information that still needs confirmation.

Provide a pin-assignment table or an equally clear description. Until RONGO Board Library version 1.0 or later exists, obtain the user's approval before making or changing any GPIO assignment. A proposal is not permission to make the change.

Do not change previously approved assignments independently. If sources conflict, report the conflict and request confirmation.

## 6. Future RONGO Board Library integration

When the board-profile library becomes available, use only profiles whose verification status is explicit and whose sources are listed.

A profile is approved only according to the library's rules; a WEP JSON report, draft, or automatically generated configuration is not approval.

If no verified profile exists for a board:

1. Report that the profile is missing.
2. Gather available evidence from reports and manufacturer documentation.
3. Separate confirmed information from unknowns.
4. Propose hardware choices and pin assignments with explanations.
5. Request approval before changing GPIO or platform configuration.

Do not add profiles to the library without separate approval and source verification.

## 7. Building and uploading firmware

Use the PlatformIO installation available in the project and first verify the actual command and environment names.

Examples, to be used only after confirming the environment name in `platformio.ini`:

```powershell
pio run -e <environment>
pio run -e <environment> -t upload
pio device monitor
```

Before uploading, check the selected board, environment, and available serial port. Uploading changes the state of physical hardware; if the user's request does not clearly authorize an upload, ask first.

If `pio` is not found in `PATH`, check whether PlatformIO is available by another installed method before reporting it as missing.

Always report the actual result and environment. A successful compile confirms only the build, not operation of the physical board.

## 8. LittleFS and filesystem uploads

If firmware serves web files from LittleFS, check that source files exist, the filesystem is configured, the partition layout is suitable, and the upload target is supported by the selected environment.

Firmware upload and filesystem-image upload are separate operations. Do not claim that web files reached the device if only firmware was uploaded.

Example command for a confirmed environment:

```powershell
pio run -e <environment> -t uploadfs
```

Before uploading the filesystem, verify the selected environment and flash partitions. Do not change flash layout or partition tables without explanation and approval. When diagnosing a missing page, check that files were uploaded to LittleFS as well as checking web-server routes.

## 9. Verifying results

Distinguish and explicitly report four verification levels:

1. **Code inspection** — source and configuration reviewed.
2. **Build** — a specific environment compiled successfully.
3. **Upload** — firmware or filesystem image was written to the device.
4. **Hardware validation** — functionality was tested on a physical board.

Do not claim a higher level based on a lower one. When multiple environments are involved, list exactly which ones were built. For hardware validation, identify the exact board and functions actually tested. If the device was unavailable, state that hardware validation was not performed.

## 10. Explaining technical decisions

When changing code or configuration, explain to the user:

1. What was added or changed.
2. Why this implementation was chosen.
3. How the solution works, step by step.
4. How it interacts with the other RONGO components.
5. Which functions, variables, and data structures are involved.
6. What limitations, conflicts, and failure modes may occur.
7. How the user can independently verify the result.

Explain the consequences of changes to `platformio.ini`, environments, GPIO, ESP32 families, flash/PSRAM, dependencies, and upload commands in particular. Do not provide only finished code without enough explanation for the user to understand and verify it independently.

## 11. Agent authority limits

- Do not change GPIO without the approval required by section 5.
- Do not choose between conflicting hardware claims on your own.
- Do not treat a WEP report as an approved board profile or allow it to automatically replace the main `platformio.ini`.
- Do not change flash layout, the active environment, or dependencies without a technical justification and approval.
- Do not upload to physical hardware without clear user authorization.
- Do not present a successful build as proof of hardware operation.
- Do not invent profiles or attribute unverified properties to a board.
- Do not implement WEP functionality inside RONGO.
- When you find contradictory information, report it first, then request the missing data or approval.
- Do not modify unrelated files or expand the task without the user's request.
