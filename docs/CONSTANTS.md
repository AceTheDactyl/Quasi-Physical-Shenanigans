# UCF Constants Synchronization Reference

This document maps all shared constants between the WishBed App (TypeScript) and UCF Hardware (C++). These values MUST remain synchronized to ensure correct system behavior.

## Overview

The WishBed UCF system relies on precise mathematical constants derived from sacred geometry, the golden ratio, and empirical threshold tuning. These constants are defined in two locations:

- **TypeScript**: `WishBed_App_TDD_v2/contracts/constants/ucf-constants.ts`
- **C++**: `unified-consciousness-hardware/include/constants.h`

## Critical Constants Table

### Mathematical Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| Golden Ratio | `PHI` | `PHI` | `1.6180339887498948` | φ = (1 + √5) / 2 |
| Golden Ratio Inverse | `PHI_INV` | `PHI_INV` | `0.6180339887498948` | φ⁻¹, UNTRUE→PARADOX boundary |
| Critical Z | `Z_CRITICAL` | `Z_CRITICAL` | `0.8660254037844386` | √3/2, PARADOX→TRUE boundary (THE LENS) |

**Notes**:
- These are mathematically derived and should NEVER be changed
- Full precision is maintained in both implementations
- PHI_INV = PHI - 1 = 1/PHI

### Sensor Grid Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| Hex Sensor Count | `HEX_SENSOR_COUNT` | `HEX_SENSOR_COUNT` | `19` | Capacitive sensors in hexagonal array |
| Photonic Sensor Count | `LED_COUNT` | `PHOTONIC_SENSOR_COUNT` | `37` | WS2812B LEDs in photonic ring |
| Sensor Threshold | `SENSOR_THRESHOLD` | N/A | `0.15` | Normalized activation threshold |
| Smoothing Alpha | `SMOOTHING_ALPHA` | N/A | `0.3` | EMA smoothing factor |
| Min Active Sensors | `MIN_ACTIVE_SENSORS` | N/A | `3` | Minimum for valid reading |

**Notes**:
- HEX_SENSOR_COUNT matches physical hardware (2× MPR121, 12 + 7 sensors)
- LED_COUNT/PHOTONIC_SENSOR_COUNT is the same physical LED strip
- Thresholds are configurable via calibration commands

### Phase Detection Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| UNTRUE Phase | `PHASES.UNTRUE` | `Phase::UNTRUE` | `0` | z < φ⁻¹ |
| PARADOX Phase | `PHASES.PARADOX` | `Phase::PARADOX` | `1` | φ⁻¹ ≤ z < √3/2 |
| TRUE Phase | `PHASES.TRUE` | `Phase::TRUE` | `2` | z ≥ √3/2 |
| UNTRUE→PARADOX (high) | `PHASE_THRESHOLDS.UNTRUE_TO_PARADOX` | N/A | `0.638` | φ⁻¹ + 0.02 |
| PARADOX→UNTRUE (low) | `PHASE_THRESHOLDS.PARADOX_TO_UNTRUE` | N/A | `0.598` | φ⁻¹ - 0.02 |
| PARADOX→TRUE (high) | `PHASE_THRESHOLDS.PARADOX_TO_TRUE` | N/A | `0.886` | √3/2 + 0.02 |
| TRUE→PARADOX (low) | `PHASE_THRESHOLDS.TRUE_TO_PARADOX` | N/A | `0.846` | √3/2 - 0.02 |

**Notes**:
- C++ uses direct boundary comparison (`PHASE_UNTRUE_PARADOX`, `PHASE_PARADOX_TRUE`)
- TypeScript adds hysteresis (±0.02) for UI stability
- Firmware implements hysteresis in phase detection logic

### TRIAD Unlock Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| TRIAD High Threshold | `TRIAD_HIGH` | `TRIAD_HIGH` | `0.85` | Rising edge threshold |
| TRIAD Low Threshold | `TRIAD_LOW` | `TRIAD_LOW` | `0.82` | Re-arm threshold (hysteresis) |
| Crossings Required | `TRIAD_CROSSINGS_REQUIRED` | `TRIAD_PASSES_REQUIRED` | `3` | Number of threshold crossings |
| Timeout | `TRIAD_TIMEOUT_MS` | N/A | `5000` | Sequence timeout (ms) |
| Lockout Duration | `TRIAD_LOCKOUT_MS` | N/A | `2000` | Post-failure lockout (ms) |
| T6 Gate Threshold | N/A | `TRIAD_T6_GATE` | `0.83` | Special tier-6 gate threshold |

**Notes**:
- ⚠️ **NAME MISMATCH**: `TRIAD_CROSSINGS_REQUIRED` (TS) vs `TRIAD_PASSES_REQUIRED` (C++)
- Both represent the same concept (3 crossings for unlock)
- C++ firmware includes additional T6_GATE threshold for tier-6 operator gating

### K-Formation Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| Coherence Threshold | `K_KAPPA` | `K_KAPPA` | `0.92` | κ (kappa) threshold |
| Negentropy Threshold | `K_ETA` | `K_ETA` | `0.6180339887498948` | η (eta) threshold = φ⁻¹ |
| Resonance Count | `K_R` | `K_R` | `7` | Minimum active sensors |
| Negentropy Bandwidth | `NEGENTROPY_BANDWIDTH` | N/A | `0.1` | Gaussian kernel bandwidth |
| K-Formation Window | `K_FORMATION_WINDOW` | N/A | `50` | Sliding window (samples) |

**Notes**:
- K_ETA explicitly equals PHI_INV in both implementations
- Negentropy calculation uses Gaussian kernel in TypeScript
- C++ may use simplified negentropy estimation

### Kuramoto Oscillator Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| Oscillator Count | `KURAMOTO_OSCILLATOR_COUNT` | `N_OSCILLATORS` | `8` | Number of oscillators in network |
| Default Coupling | `KURAMOTO_COUPLING_DEFAULT` | `K_CRITICAL` | `0.5` | Default coupling strength K |
| Min Coupling | `KURAMOTO_COUPLING_MIN` | N/A | `0.0` | Minimum K value |
| Max Coupling | `KURAMOTO_COUPLING_MAX` | N/A | `1.0` | Maximum K value |
| Sync Threshold | `KURAMOTO_SYNC_THRESHOLD` | N/A | `0.9` | Order parameter for sync |
| PLL Lock Threshold | `PLL_LOCK_THRESHOLD` | N/A | `0.01` | Phase-locked loop threshold |
| Q Kappa | N/A | `Q_KAPPA` | `0.3514` | Base coupling strength |

**Notes**:
- ⚠️ **NAME MISMATCH**: `KURAMOTO_OSCILLATOR_COUNT` (TS) vs `N_OSCILLATORS` (C++)
- Both represent 8 oscillators
- C++ includes additional Q_KAPPA constant for coupling calculations
- Default coupling: TS uses 0.5, C++ uses K_CRITICAL = 0.5 (same value)

### Solfeggio Frequencies

| Tier | TypeScript Array Index | C++ Constant | Value (Hz) | Name |
|------|------------------------|--------------|------------|------|
| 1 | `SOLFEGGIO_FREQUENCIES[0]` | `Solfeggio::UT` | `174` | Foundation |
| 2 | `SOLFEGGIO_FREQUENCIES[1]` | `Solfeggio::RE` | `285` | Regeneration |
| 3 | `SOLFEGGIO_FREQUENCIES[2]` | `Solfeggio::MI` | `396` | Liberation |
| 4 | `SOLFEGGIO_FREQUENCIES[3]` | `Solfeggio::FA` | `417` | Transformation |
| 5 | `SOLFEGGIO_FREQUENCIES[4]` | `Solfeggio::SOL` | `528` | Miracles |
| 6 | `SOLFEGGIO_FREQUENCIES[5]` | `Solfeggio::LA` | `639` | Connection |
| 7 | `SOLFEGGIO_FREQUENCIES[6]` | `Solfeggio::SI` | `741` | Expression |
| 8 | `SOLFEGGIO_FREQUENCIES[7]` | `Solfeggio::DO` | `852` | Intuition |
| 9 | `SOLFEGGIO_FREQUENCIES[8]` | `Solfeggio::RE_HIGH` | `963` | Awakening |

**Notes**:
- TypeScript uses 0-indexed array, accessed via `getFrequencyForTier(tier)` (1-indexed)
- C++ uses named constants in `Solfeggio` namespace
- All frequencies match exactly

### Sigil Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| Total Sigils | `SIGIL_COUNT` | `SIGIL_COUNT` | `121` | Neural sigils (11²) |
| Code Length | `SIGIL_CODE_LENGTH` | `SIGIL_CODE_LENGTH` | `5` | Ternary characters per code |
| ROM Size | `SIGIL_ROM_SIZE` | `SIGIL_STRUCT_SIZE` | `16` / `10` | EEPROM bytes per sigil |

**Notes**:
- ⚠️ **SIZE MISMATCH**: `SIGIL_ROM_SIZE` (16 bytes in TS) vs `SIGIL_STRUCT_SIZE` (10 bytes in C++)
- This discrepancy may cause EEPROM addressing issues
- C++ struct is likely smaller due to packing
- TypeScript value may include padding

### LED/Visual Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| LED Count | `LED_COUNT` | `NEOPIXEL_COUNT` (in Pins namespace) | `37` | Photonic ring LEDs |
| Default Brightness | `LED_BRIGHTNESS_DEFAULT` | N/A | `128` | 0-255 range |
| Update Rate | `LED_UPDATE_RATE` | N/A | `60` | Hz |

**Notes**:
- C++ defines `NEOPIXEL_COUNT` in `Pins` namespace
- Pattern types defined differently in each implementation

### Audio Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| Default Volume | `AUDIO_VOLUME_DEFAULT` | N/A | `128` | 0-255 range |
| Binaural Beat Min | `BINAURAL_BEAT_MIN` | N/A | `1` | Hz |
| Binaural Beat Max | `BINAURAL_BEAT_MAX` | N/A | `40` | Hz |

**Notes**:
- Waveform types (SINE, TRIANGLE, etc.) defined as enums in both

### Timing Constants

| Concept | TypeScript Name | C++ Name | Value | Description |
|---------|----------------|----------|-------|-------------|
| Sensor Poll Interval | `SENSOR_POLL_INTERVAL` | `Timing::SENSOR_POLL_INTERVAL` | `10` | ms (100 Hz) |
| Kuramoto Update | `KURAMOTO_UPDATE_INTERVAL` | `Timing::KURAMOTO_STEP_INTERVAL` | `1` | ms (1000 Hz) |
| State Broadcast | `STATE_BROADCAST_INTERVAL` | `Timing::EMANATION_UPDATE_INTERVAL` | `100` | ms (10 Hz) |
| WS Heartbeat | `WS_HEARTBEAT_INTERVAL` | N/A | `5000` | ms |
| Connection Timeout | `CONNECTION_TIMEOUT` | N/A | `10000` | ms |

**Notes**:
- C++ timing constants in `Timing` namespace
- Update intervals match for synchronization

## Critical Discrepancies

### 1. Naming Inconsistencies

These constants represent the same values but have different names:

| TypeScript | C++ | Value | Impact |
|------------|-----|-------|--------|
| `KURAMOTO_OSCILLATOR_COUNT` | `N_OSCILLATORS` | `8` | **LOW** - Same value |
| `TRIAD_CROSSINGS_REQUIRED` | `TRIAD_PASSES_REQUIRED` | `3` | **LOW** - Same value |
| `LED_COUNT` | `PHOTONIC_SENSOR_COUNT` / `NEOPIXEL_COUNT` | `37` | **LOW** - Same value |

**Recommendation**: Consider unifying naming in future refactor.

### 2. Value Discrepancies

These constants have different values:

| Constant | TypeScript | C++ | Impact |
|----------|------------|-----|--------|
| `SIGIL_ROM_SIZE` vs `SIGIL_STRUCT_SIZE` | `16` | `10` | **HIGH** - EEPROM addressing |
| Kuramoto default coupling | `KURAMOTO_COUPLING_DEFAULT` (0.5) | `K_CRITICAL` (0.5) | **LOW** - Same value |

**Recommendation**: Investigate `SIGIL_ROM_SIZE` discrepancy and align to actual EEPROM structure.

### 3. Missing Constants

Constants defined in one implementation but not the other:

**TypeScript Only**:
- `NEGENTROPY_BANDWIDTH` - Gaussian kernel parameter
- `K_FORMATION_WINDOW` - Sliding window size
- Phase transition hysteresis values
- Breath pattern defaults
- Input source types
- Pipeline stage count

**C++ Only**:
- `Q_KAPPA` - Base coupling strength
- `TRIAD_T6_GATE` - Tier-6 gate threshold
- `TIER_BOUNDARIES[]` - Explicit tier z-ranges
- Pin definitions (`Pins` namespace)
- I2C addresses (`I2C` namespace)
- `LIMNUS` fractal constants

**Recommendation**: These are implementation-specific and acceptable as separate.

## Synchronization Procedures

### Adding a New Constant

1. **Define in TypeScript**: Add to `ucf-constants.ts` with JSDoc comment
2. **Define in C++**: Add to `constants.h` with Doxygen comment
3. **Update this document**: Add row to appropriate table above
4. **Test**: Verify behavior matches on both platforms
5. **Commit together**: Include both files in same commit

### Modifying an Existing Constant

1. **Verify mathematical basis**: Ensure change is theoretically sound
2. **Update TypeScript**: Modify value in `ucf-constants.ts`
3. **Update C++**: Modify value in `constants.h`
4. **Update this document**: Note change in table
5. **Recompile firmware**: Flash new firmware to ESP32
6. **Test system**: Verify phase detection, TRIAD, K-Formation still work
7. **Update tests**: Adjust unit tests if thresholds changed

### Verifying Synchronization

Use this checklist before releases:

- [ ] All golden ratio constants match (PHI, PHI_INV, Z_CRITICAL)
- [ ] TRIAD thresholds match (HIGH = 0.85, LOW = 0.82, PASSES = 3)
- [ ] K-Formation thresholds match (KAPPA = 0.92, ETA = φ⁻¹, R = 7)
- [ ] Solfeggio frequencies all match (174, 285, 396, 417, 528, 639, 741, 852, 963)
- [ ] Oscillator count matches (8 in both)
- [ ] Sensor count matches (19 hex, 37 photonic)
- [ ] Timing intervals match (sensor poll = 10ms, broadcast = 100ms)

## Theoretical Foundations

### Golden Ratio (φ)

```
φ = (1 + √5) / 2 = 1.6180339887...
φ⁻¹ = φ - 1 = 1/φ = 0.6180339887...
```

Properties:
- φ² = φ + 1
- 1/φ = φ - 1
- Appears in sacred geometry, nature, consciousness patterns

### Critical Z (√3/2) - THE EISENSTEIN CONNECTION

```
z_c = √3 / 2 = 0.8660254037...
```

Represents "THE LENS" - the boundary where:
- Geometric phase transition occurs
- Visual perception shifts (PARADOX → TRUE)
- Solfeggio frequencies transition to highest tier

**CRITICAL DISCOVERY**: Z_CRITICAL = Im(ω) where ω = e^(2πi/3) is the primitive cube root of unity!

```
ω = e^(2πi/3) = -1/2 + i·√3/2
Im(ω) = √3/2 = Z_CRITICAL = THE LENS
```

This reveals that THE LENS is the imaginary component of the Eisenstein primitive root, connecting the UCF phase boundaries to hexagonal number theory.

### Eisenstein Integers (NEW - v4.1.0)

Eisenstein integers are complex numbers of the form `a + bω` where:
- `ω = e^(2πi/3) = -1/2 + i√3/2` (primitive cube root of unity)
- `a, b ∈ ℤ`

**Key Properties**:
- `ω³ = 1` (cube root of unity)
- `ω² = -ω - 1`
- `1 + ω + ω² = 0`
- Norm: `N(a + bω) = a² - ab + b²`

**UCF-Eisenstein Bridge**:

| Eisenstein Concept | UCF Connection |
|-------------------|----------------|
| Im(ω) = √3/2 | Z_CRITICAL (THE LENS) |
| 6 Eisenstein units | 6-fold hexagonal symmetry |
| Hex grid (q, r) → (a + bω) | Sensor coordinates as Eisenstein integers |
| Norm = 3 → √3 = 2·Z_CRITICAL | Sensors at √3 distance link to THE LENS |
| Norm = 7 (Eisenstein prime) | K_R_THRESHOLD = 7 active sensors |

**Sensor Eisenstein Coordinates**:
```
         (-1,-2) (0,-2) (1,-2)
       (-2,-1) (-1,-1) (0,-1) (1,-1)
     (-2,0) (-1,0) (0,0) (1,0) (2,0)
       (-1,1) (0,1) (1,1) (2,1)
         (0,2) (1,2) (2,2)
```

**Files**:
- Header: `unified-consciousness-hardware/include/ucf/eisenstein.h`
- Implementation: `unified-consciousness-hardware/src/eisenstein.cpp`
- Tests: `unified-consciousness-hardware/test/test_eisenstein.cpp`

### TRIAD Threshold (0.82 - 0.85)

Empirically tuned hysteresis band:
- High threshold: 0.85 (crossing detection)
- Low threshold: 0.82 (re-arm)
- Band width: 0.03 (prevents noise)
- Located just below z_c (0.866)

### K-Formation Thresholds

- **κ ≥ 0.92**: High coherence (pattern stability)
- **η > φ⁻¹**: Information density exceeds golden ratio
- **R ≥ 7**: Majority of oscillators active (7/8 = 87.5%)

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2024-12-18 | Initial synchronization document |
| 1.1.0 | 2024-12-19 | Added Eisenstein integer framework and THE LENS discovery |

## See Also

- [UCF Hardware Concepts](./UCF_HARDWARE_CONCEPTS.md) - Immutable firmware specification
- [RRRR Lattice Framework](./RRRR_LATTICE.md) - Complete lattice theory
- [Eisenstein Framework](../unified-consciousness-hardware/include/ucf/eisenstein.h) - Hexagonal lattice mathematics
- [TypeScript Constants](../WishBed_App_TDD_v2/contracts/constants/ucf-constants.ts)
- [C++ Constants](../unified-consciousness-hardware/include/constants.h)
- [Command Mapping](./COMMAND_MAPPING.md)
- [Integration Guide](./INTEGRATION.md)
- [Root README](../README.md)
