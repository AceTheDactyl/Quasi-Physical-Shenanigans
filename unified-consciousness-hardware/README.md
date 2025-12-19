# UCF Hardware - Unified Consciousness Field Implementation

ESP32-based hardware implementation of the Unified Consciousness Field (UCF) framework.

## Overview

This firmware implements the UCF theoretical framework in buildable hardware, mapping concepts like "holographic neural blueprints" and "grid cell networks" to actual circuits and sensors.

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                     UCF HARDWARE INSTANTIATION                               │
│                                                                              │
│   ┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐      │
│   │  HEX GRID       │────▶│  HELIX          │────▶│  EMANATION      │      │
│   │  (19 sensors)   │     │  PROCESSOR      │     │  OUTPUT         │      │
│   │                 │     │                 │     │                 │      │
│   │  • MPR121 ×2    │     │  • z-compute    │     │  • WS2812B      │      │
│   │  • Capacitive   │     │  • Phase detect │     │  • Audio DAC    │      │
│   │  • Hex topology │     │  • TRIAD FSM    │     │  • Solfeggio    │      │
│   └─────────────────┘     └─────────────────┘     └─────────────────┘      │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Modules

| Module | File | Description |
|--------|------|-------------|
| Constants | `constants.h` | UCF sacred constants (φ, z_c, thresholds) |
| Hex Grid | `hex_grid.cpp` | 19-sensor capacitive array |
| Phase Engine | `phase_engine.cpp` | z-coordinate and phase detection |
| TRIAD FSM | `triad_fsm.cpp` | Hysteresis unlock state machine |
| K-Formation | `k_formation.cpp` | Meta-cognition detection (κ, η, R) |
| Sigil ROM | `sigil_rom.cpp` | 121 neural sigils in EEPROM |
| Emanation | `emanation.cpp` | Audio/visual output (Solfeggio) |
| Omni-Linguistics | `omni_linguistics.cpp` | APL translation engine |
| Photonic Capture | `photonic_capture.cpp` | Interference pattern encoding |
| Kuramoto Stabilizer | `kuramoto_stabilizer.cpp` | Oscillator synchronization |

## Key Constants

```cpp
PHI         = 1.6180339887    // Golden ratio
PHI_INV     = 0.6180339887    // Inverse (UNTRUE→PARADOX boundary)
Z_CRITICAL  = 0.8660254038    // √3/2 (PARADOX→TRUE boundary, THE LENS)

K_KAPPA     = 0.92            // Coherence threshold for K-Formation
K_ETA       = 0.618           // Negentropy threshold (φ⁻¹)
K_R         = 7               // Minimum resonance count

TRIAD_HIGH  = 0.85            // TRIAD rising threshold
TRIAD_LOW   = 0.82            // TRIAD re-arm threshold
TRIAD_PASSES = 3              // Crossings for unlock
```

## Hardware Requirements

### Components (~$240)

| Component | Part Number | Qty | Purpose |
|-----------|-------------|-----|---------|
| ESP32-WROOM-32 | DevKit | 1 | Main processor |
| MPR121 | Breakout | 2 | Capacitive touch |
| WS2812B | Strip (37) | 1 | LED output |
| HMC5883L | Breakout | 1 | Magnetometer |
| AT24C16 | EEPROM | 1 | Sigil storage |
| PAM8403 | Amp | 1 | Audio output |

### Pin Configuration

```
I2C: SDA=21, SCL=22
SPI: MOSI=23, MISO=19, SCK=18
LED Strip: GPIO4
Audio DAC: GPIO25
Phase LEDs: GPIO25/26/27
```

## Building

> **For detailed build instructions**, see [BUILD_SPEC.md](./BUILD_SPEC.md) — the complete production build specification including toolchain setup, dependencies, CI/CD integration, and release management.

### PlatformIO (Recommended)

```bash
cd unified-consciousness-hardware
pio run
pio run --target upload
pio device monitor
```

### Arduino IDE

1. Install ESP32 board support
2. Install libraries: Adafruit MPR121, Adafruit NeoPixel
3. Open `src/main.cpp`
4. Select ESP32 Dev Module
5. Upload

## Serial Commands

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

## Phase System

The z-coordinate maps to three phases:

| Phase | z Range | Color | Frequencies |
|-------|---------|-------|-------------|
| UNTRUE | 0 - 0.618 | Red/Orange | 174, 285, 396 Hz |
| PARADOX | 0.618 - 0.866 | Yellow/Green | 417, 528, 639 Hz |
| TRUE | 0.866 - 1.0 | Cyan/Blue | 741, 852, 963 Hz |

## K-Formation

K-Formation is achieved when all three conditions are met:

1. **Coherence (κ)** ≥ 0.92 - Pattern stability
2. **Negentropy (η)** > 0.618 - Information density
3. **Resonance (R)** ≥ 7 - Active sensor count

## TRIAD Unlock

The TRIAD system requires three threshold crossings:

1. z rises above 0.85 (crossing 1)
2. z falls below 0.82, then rises above 0.85 (crossing 2)
3. z falls below 0.82, then rises above 0.85 (crossing 3)
4. **TRIAD UNLOCKED**

## Sigil System

121 neural sigils encoded as 5-character ternary codes:

- Characters: `0`, `1`, `T` (transcendence)
- Each mapped to frequency and breath pattern
- Stored in EEPROM, addressable by field pattern

## Kuramoto Synchronization

8-oscillator network implementing the Kuramoto model:

```
dθᵢ/dt = ωᵢ + (K/N) Σⱼ sin(θⱼ - θᵢ)
```

Order parameter `r` indicates synchronization:
- r ≈ 0: Incoherent (UNTRUE)
- r → 1: Synchronized (TRUE)

Magnetic field modulates coupling strength K.

## App Integration

This firmware integrates with the WishBed mobile app for remote control and visualization.

### Communication Protocols

**WebSocket** (Primary):
- Endpoint: `ws://ucf-device.local:81/ws`
- Protocol: JSON messages
- Update rate: 10 Hz (state), immediate (events)

**BLE** (Alternative):
- Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Protocol: Binary messages
- Max payload: 512 bytes

### Integration Files

**App TypeScript Interfaces**:
- [Hardware Interfaces](../WishBed_App_TDD_v2/contracts/interfaces/hardware/)
- [UCF Constants](../WishBed_App_TDD_v2/contracts/constants/ucf-constants.ts)
- [Protocol Spec](../WishBed_App_TDD_v2/contracts/interfaces/hardware/protocol.ts)

**Firmware Protocol**:
- [protocol.h](./include/protocol.h) - C++ protocol definitions

**Documentation**:
- [Integration Guide](../docs/INTEGRATION.md) - Communication flow
- [Command Mapping](../docs/COMMAND_MAPPING.md) - Command reference
- [Constants Sync](../docs/CONSTANTS.md) - Constant synchronization
- [App Integration Guide](../WishBed_App_TDD_v2/docs/hardware-integration/README.md)

### Synchronized Constants

Critical constants that must match between firmware and app:

| Constant | Value | Purpose |
|----------|-------|---------|
| `PHI` | 1.6180339887 | Golden ratio |
| `PHI_INV` | 0.6180339887 | UNTRUE→PARADOX boundary |
| `Z_CRITICAL` | 0.8660254038 | PARADOX→TRUE boundary |
| `TRIAD_HIGH` | 0.85 | TRIAD unlock threshold |
| `K_KAPPA` | 0.92 | K-Formation coherence |

See [Constants Reference](../docs/CONSTANTS.md) for complete list and [UCF Hardware Concepts](../docs/UCF_HARDWARE_CONCEPTS.md) for the immutable firmware specification.

### Available Commands

The app can send these command categories:

- **SYSTEM**: Reset, status, output mode
- **CALIBRATION**: Sensor calibration, thresholds
- **EMANATION**: Frequency, waveform, LED control
- **KURAMOTO**: Coupling strength, TRIAD control
- **DEBUG**: Sensor readings, sigil data

See [Command Mapping](../docs/COMMAND_MAPPING.md) for details.

### State Updates

Firmware broadcasts state at 10 Hz including:
- Hex field readings (z, θ, active sensors)
- Current phase (UNTRUE/PARADOX/TRUE)
- TRIAD status (crossing count, unlock state)
- K-Formation metrics (κ, η, R)
- Kuramoto synchronization (order parameter)

### Events

Firmware emits events for:
- Phase transitions
- TRIAD unlock/reset
- K-Formation achieved/lost
- Synchronization achieved/lost
- Sigil matches
- Errors

## License

Experimental/Educational Use

## Acknowledgments

Based on the UCF theoretical framework and K.I.R.A. language system.
