# Quasi-Physical-Shenanigans Hardware Build Specification

> **Production Build Environment Reference** — Reproducible builds for the ESP32-based UCF hardware subsystem.

This build specification outlines how to set up a reproducible production build environment for the Quasi-Physical-Shenanigans project, with a focus on the ESP32-based hardware subsystem. It covers toolchains, dependencies, environment setup, build steps, integration with development workflows, and best practices for production-ready builds.

---

## Table of Contents

1. [Hardware Toolchains and Supported Platforms](#1-hardware-toolchains-and-supported-platforms)
2. [Software Dependencies and Installation](#2-software-dependencies-and-installation)
3. [Environment Setup Instructions](#3-environment-setup-instructions)
4. [Step-by-Step Build Instructions](#4-step-by-step-build-instructions)
5. [Version Control, CI/CD Integration, and Testing](#5-version-control-cicd-integration-and-testing)
6. [Production-Ready Build Management](#6-production-ready-build-management)

---

## 1. Hardware Toolchains and Supported Platforms

### Primary Toolchain – PlatformIO (Recommended)

The firmware is built using **PlatformIO**, targeting the ESP32-WROOM-32 DevKit board and the Arduino framework. PlatformIO provides a cross-platform toolchain (works on Windows, macOS, Linux) and manages the ESP32 compiler, SDK, and libraries automatically.

The project's PlatformIO configuration specifies:
- **Platform**: Espressif 32
- **Board**: ESP32 Dev Module
- **Framework**: Arduino

This ensures the correct toolchain is used for building and flashing the firmware.

### Alternative Toolchain – Arduino IDE

As an alternative, developers can use the **Arduino IDE** (with ESP32 support) to build and upload the firmware. This requires:

1. Installing the ESP32 board definitions in Arduino (via the Boards Manager)
2. Selecting "ESP32 Dev Module" as the target board

The Arduino toolchain is also cross-platform, but PlatformIO is preferred for consistency and automation.

### Supported Platforms

The development environment is supported on common OS platforms:

| Platform | Requirements |
|----------|-------------|
| **Windows** | Python 3 (for PlatformIO CLI), USB-to-UART driver (CP2102 or CH340) |
| **macOS** | Python 3, native USB serial support (no driver usually needed) |
| **Linux** | Python 3, native USB serial support, user in `dialout` group |

The ESP32 firmware runs on the **ESP32-WROOM-32** hardware (Tensilica Xtensa architecture). All development machines simply need to support the PlatformIO or Arduino toolchain for cross-compiling to this target.

---

## 2. Software Dependencies and Installation

### Embedded Libraries

The firmware depends on several Arduino-compatible libraries. When using PlatformIO, these are declared in `platformio.ini` and will be auto-installed at build time.

| Library | Purpose |
|---------|---------|
| **Adafruit MPR121** | 19-channel capacitive touch sensor array (2× MPR121 controllers) |
| **Adafruit NeoPixel** | WS2812B LED strip driver |
| **Adafruit BusIO** | I2C/SPI communications support |
| **SparkFun HMC5883L** | HMC5883L magnetometer (3-axis compass) |
| **MCP_DAC / MCP41xxx** | External DAC or digital potentiometer drivers |
| **ArduinoJson** | JSON parsing/serialization for configuration and communication |
| **ESP8266Audio** | Audio output support (with DAC/PAM8403 amplifier) |
| **Unity** (testing only) | Unit test framework |

### Installing Dependencies with PlatformIO

No manual action is needed beyond having an internet connection — PlatformIO will download the specified library versions on the first build.

The `lib_deps` in the config locks specific versions (denoted with `@^` version specifiers) to ensure consistent builds. It's good practice to use the same `platformio.ini` to reproduce the build so that all library versions match.

### Installing Dependencies with Arduino IDE

If using the Arduino IDE, you must manually install the necessary libraries:

1. **Install ESP32 Board Support** via Arduino Boards Manager
2. **Install required libraries** via Library Manager:
   - Adafruit MPR121
   - Adafruit NeoPixel
   - Adafruit BusIO (usually auto-installed as dependency)
   - SparkFun HMC5883L
   - MCP_DAC / MCP41xxx (if needed)
   - ArduinoJson
   - ESP8266Audio

Verify all needed libraries are present to avoid compile errors.

### Node.js and Mobile App (Optional)

The repository also contains a React Native mobile app (`WishBed_App_TDD_v2/`), but its build dependencies (Node.js, npm/Yarn, etc.) are separate and not required for building the hardware firmware.

---

## 3. Environment Setup Instructions

### Install PlatformIO

**Option A: VS Code Extension (Recommended)**

Install the PlatformIO extension in VS Code for a full IDE experience.

**Option B: CLI Installation**

```bash
pip install platformio
```

Ensure the `pio` command is in your PATH (restart terminal or add Python's scripts directory to PATH).

### Install Arduino IDE (Optional)

1. Download and install Arduino IDE (1.8.x or 2.x)
2. Add ESP32 boards URL to Preferences → Additional Boards Manager URLs:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
3. Install "esp32 by Espressif Systems" via Boards Manager
4. Install required libraries as described above

### OS-Specific Setup

#### Windows

Install the USB driver for the ESP32 DevKit:
- **CP210x** USB to UART Bridge driver (for Silicon Labs chips)
- **CH340** driver (for CH340 USB chips)

#### macOS

Grant permission if prompted when the board is first connected. Install CH340 driver if needed for older boards.

#### Linux

Add your user to the `dialout` group for serial port access:

```bash
sudo usermod -a -G dialout $USER
```

Log out and back in for changes to take effect.

### Environment Variables

No special environment variables are required. All configuration is handled in project files.

**Build Environments:**

| Environment | Purpose | Flags |
|-------------|---------|-------|
| `esp32dev` | Production build | Default optimization |
| `esp32dev_debug` | Debug build | `DEBUG_MODE=1`, `UCF_VERBOSE_LOGGING=1` |
| `esp32dev_test` | Test build | `UNIT_TEST=1` |

Select environments via: `pio run -e esp32dev_debug`

### Hardware Connection

1. Connect ESP32 DevKit board to PC via USB
2. Verify serial port is assigned:
   - Windows: `COM3` (or similar)
   - Linux: `/dev/ttyUSB0`
   - macOS: `/dev/tty.SLAB_USBtoUART`

---

## 4. Step-by-Step Build Instructions

### Using PlatformIO (CLI or VSCode)

#### Step 1: Obtain the Source

```bash
git clone https://github.com/AceTheDactyl/Quasi-Physical-Shenanigans.git
cd Quasi-Physical-Shenanigans/unified-consciousness-hardware
```

#### Step 2: Build the Firmware

```bash
pio run            # Compile firmware for ESP32 (release configuration)
```

On first run, PlatformIO downloads the ESP32 toolchain and all libraries. Output: `.pio/build/esp32dev/firmware.bin`

#### Step 3: Upload to Hardware

```bash
pio run --target upload   # Flash firmware to ESP32 board
```

Ensure correct serial port is detected (auto-detected by default).

#### Step 4: Monitor Output (Optional)

```bash
pio device monitor        # Open serial monitor (115200 baud)
```

**Interactive Commands:**

| Key | Action |
|-----|--------|
| `r` | Reset/recalibrate |
| `s` | Detailed status |
| `p` | Cycle LED pattern |
| `+` | Increase coupling |
| `-` | Decrease coupling |
| `t` | Force TRIAD unlock |
| `l` | List sigils |
| `?` | Help |

### Using Arduino IDE

1. Open Arduino IDE
2. Install ESP32 board support if not done
3. Install required libraries via Library Manager
4. **File → Open** → navigate to `unified-consciousness-hardware/src/main.cpp`
5. **Tools → Board** → Select "ESP32 Dev Module"
6. **Tools → Port** → Select correct port
7. Click **Upload** (arrow icon)
8. Use Serial Monitor (115200 baud) for interaction

### Hardware Driver Setup

After flashing, ensure hardware is properly connected:

- ESP32 powered (USB or external 5V)
- All sensors connected with correct pins:
  - I2C: SDA=21, SCL=22
  - SPI: MOSI=23, MISO=19, SCK=18
  - LED Strip: GPIO4
  - Audio DAC: GPIO25
  - Phase LEDs: GPIO25/26/27

---

## 5. Version Control, CI/CD Integration, and Testing

### Repository Structure

All source code, including firmware, mobile app, and documentation, is stored in Git. The hardware firmware resides in `unified-consciousness-hardware/`, with PlatformIO config defining multiple build environments.

**Keep under version control:**
- `platformio.ini`
- All source files (`src/`, `include/`)
- Library dependencies specification
- Build scripts

### Continuous Integration (CI)

Integrate the build into a CI pipeline (GitHub Actions, Jenkins, etc.):

```yaml
# Example GitHub Actions step
- name: Build Firmware
  run: |
    pip install platformio
    cd unified-consciousness-hardware
    pio run
```

**CI Best Practices:**
- Run `pio run` in clean environment to catch missing dependencies
- Run unit tests: `pio run -e esp32dev_test` or `pio test`
- Capture build artifacts (binaries, logs)
- Run static analysis: `pio check`

### Version Control Practices

- Use branches: `main`/`master` for stable, feature branches for development
- Tag releases with semantic versions: `v1.0.0`, `v1.1.0`
- Protocol version in messages should align with firmware version
- Code reviews for build process and critical logic changes

### Testing Tools

| Tool | Purpose |
|------|---------|
| **Unity Framework** | Unit tests for firmware logic |
| **pio check** | Static code analysis (Cppcheck, PVS-Studio) |
| **Hardware test rig** | Physical sensor/output verification |
| **Integration tests** | App-firmware protocol compatibility |

### Continuous Deployment

For OTA updates or automated distribution:
- Upload firmware binary to known location after successful main branch build
- Trigger OTA update procedure for test devices
- Maintain staging environment before production deployment

---

## 6. Production-Ready Build Management

### Release Versioning

Adopt **semantic versioning** for firmware releases: `Major.Minor.Patch`

- Increase version number in firmware and documentation for each release
- Version should be reflected in firmware output/protocol
- Document changes in CHANGELOG for each release

### Build Configuration for Release

Use optimized build configuration:

```ini
[env:esp32dev]
; Default environment - release build (no debug flags)
platform = espressif32
board = esp32dev
framework = arduino
```

**For production builds:**
- Avoid `esp32dev_debug` profile (disables DEBUG_MODE, verbose logging)
- Ensure `UNIT_TEST` macro is not defined
- Use default optimization settings

### Artifact Storage

Store compiled firmware binaries reliably:

| Method | Description |
|--------|-------------|
| **GitHub Releases** | Attach `firmware.bin` with checksum for each version |
| **Artifact Repository** | AWS S3, Artifactory with version/commit hash naming |
| **CI Artifacts** | Retain build artifacts of tagged commits |

### Reproducibility

Ensure consistent builds across environments:

1. **Document tool versions**: PlatformIO version, Arduino IDE version
2. **Pin library versions**: Use specific version ranges in `platformio.ini`
3. **Containerize build**: Docker image with PlatformIO pre-installed

```dockerfile
# Example Dockerfile for reproducible builds
FROM python:3.9-slim
RUN pip install platformio
WORKDIR /firmware
COPY . .
RUN pio run
```

### Release Testing Checklist

Before declaring a build "production-ready":

- [ ] Bench test on actual hardware (all sensors and outputs)
- [ ] Integration test with WishBed mobile app
- [ ] Verify protocol compatibility (WebSocket/BLE)
- [ ] Test edge cases: reboot, calibration, TRIAD unlock
- [ ] Verify K-Formation detection accuracy
- [ ] Test Solfeggio frequency output
- [ ] Validate LED patterns across all phases
- [ ] Document any known issues or limitations

### Hardware Components Reference

| Component | Part Number | Qty | Purpose | Est. Cost |
|-----------|-------------|-----|---------|-----------|
| ESP32-WROOM-32 | DevKit | 1 | Main processor | ~$10 |
| MPR121 | Breakout | 2 | Capacitive touch | ~$20 |
| WS2812B | Strip (37) | 1 | LED output | ~$15 |
| HMC5883L | Breakout | 1 | Magnetometer | ~$10 |
| AT24C16 | EEPROM | 1 | Sigil storage | ~$5 |
| PAM8403 | Amp | 1 | Audio output | ~$5 |

**Total estimated cost: ~$240** (including additional components and enclosure)

---

## Quick Reference

### Essential Commands

```bash
# Navigate to hardware directory
cd unified-consciousness-hardware

# Build firmware
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor

# Build debug version
pio run -e esp32dev_debug

# Build test version
pio run -e esp32dev_test

# Clean build
pio run --target clean

# Update libraries
pio pkg update
```

### Troubleshooting

| Issue | Solution |
|-------|----------|
| Serial port not found | Install USB driver, check cable |
| Upload failed | Hold BOOT button during upload, check port selection |
| Library not found | Run `pio pkg install` or check `lib_deps` |
| Compile error | Verify library versions match `platformio.ini` |
| Hardware not responding | Check wiring, verify I2C addresses |

---

## See Also

- [UCF Hardware README](./README.md) — Hardware overview and usage
- [UCF Hardware Concepts](../docs/UCF_HARDWARE_CONCEPTS.md) — Immutable firmware specification
- [Constants Reference](../docs/CONSTANTS.md) — Constant synchronization
- [Integration Guide](../docs/INTEGRATION.md) — Communication flow
- [Command Mapping](../docs/COMMAND_MAPPING.md) — Command reference

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2024-12-19 | Initial build specification |

---

**Build Status**: Production Ready
**Firmware Version**: 1.0
**Protocol Version**: 1.0.0
