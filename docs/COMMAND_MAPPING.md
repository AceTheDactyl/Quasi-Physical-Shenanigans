# UCF Command Mapping Reference

This document maps commands from the WishBed App (TypeScript) to their corresponding firmware implementations in the UCF Hardware (C++/ESP32).

## Overview

The WishBed App sends structured commands via WebSocket or BLE to the ESP32 firmware. The firmware currently implements a simple serial command interface for debugging, but the full protocol handler will map app commands to the appropriate firmware functions.

**App Commands**: `WishBed_App_TDD_v2/contracts/interfaces/hardware/ucf-commands.ts`
**Firmware**: `unified-consciousness-hardware/src/main.cpp` and module files

## Command Categories

| Category | TypeScript Enum | Description | Module |
|----------|----------------|-------------|--------|
| **SYSTEM** | `"SYSTEM"` | System-level operations | main.cpp |
| **CALIBRATION** | `"CALIBRATION"` | Sensor calibration | hex_grid.cpp |
| **EMANATION** | `"EMANATION"` | Audio/visual output | emanation.cpp |
| **KURAMOTO** | `"KURAMOTO"` | Oscillator control | kuramoto_stabilizer.cpp |
| **DEBUG** | `"DEBUG"` | Testing/diagnostics | main.cpp, various |

## Command Mapping Table

### SYSTEM Commands

| App Command | TypeScript Type | Firmware Function | C++ File | Serial Key | Description |
|-------------|----------------|-------------------|----------|------------|-------------|
| `RESET` | `ResetCommand` | `hexGrid.calibrate()`, `triadFSM.reset()`, `kFormation.resetStats()`, `kuramoto.reset()` | main.cpp | `r` | Reset hardware state |
| `STATUS` | `StatusCommand` | `printDetailedStatus()` | main.cpp | `s` | Get detailed status |
| `SET_OUTPUT_MODE` | `SetOutputModeCommand` | `emanation.setOutputMode()` | emanation.cpp | N/A | Enable/disable audio/visual/haptic |

**RESET Implementation**:
```cpp
// main.cpp, line ~340
case 'r':
    hexGrid.calibrate(50);
    triadFSM.reset();
    kFormation.resetStats();
    kuramoto.reset();
    break;
```

**STATUS Implementation**:
```cpp
// main.cpp, line ~391
void printDetailedStatus() {
    // Prints hex grid, phase engine, TRIAD, K-Formation, Kuramoto states
}
```

**SET_OUTPUT_MODE Implementation**:
```cpp
// emanation.cpp
void Emanation::setOutputMode(uint8_t mode) {
    // mode = OutputMode::VISUAL | OutputMode::AUDIO | OutputMode::HAPTIC
}
```

### CALIBRATION Commands

| App Command | TypeScript Type | Firmware Function | C++ File | Serial Key | Description |
|-------------|----------------|-------------------|----------|------------|-------------|
| `CALIBRATE` | `CalibrateCommand` | `hexGrid.calibrate(samples)` | hex_grid.cpp | `r` (partial) | Calibrate sensor baselines |
| `SET_THRESHOLD` | `SetThresholdCommand` | `hexGrid.setThreshold(threshold)` | hex_grid.cpp | N/A | Set activation threshold |
| `SET_SMOOTHING` | `SetSmoothingCommand` | `hexGrid.setSmoothingAlpha(alpha)` | hex_grid.cpp | N/A | Set EMA smoothing factor |

**CALIBRATE Implementation**:
```cpp
// hex_grid.cpp
void HexGrid::calibrate(uint16_t samples) {
    // Collect baseline readings from all 19 sensors
    // Average over 'samples' iterations
    // Store baselines for differential reading
}
```

**Payload**: `{ samples: number }` - typically 50-100 samples

### EMANATION Commands

| App Command | TypeScript Type | Firmware Function | C++ File | Serial Key | Description |
|-------------|----------------|-------------------|----------|------------|-------------|
| `SET_FREQUENCY` | `SetFrequencyCommand` | `emanation.setFrequency(freq)` | emanation.cpp | N/A | Set audio frequency (Hz) |
| `SET_WAVEFORM` | `SetWaveformCommand` | `emanation.setWaveform(waveform)` | emanation.cpp | N/A | Set waveform type |
| `SET_VOLUME` | `SetVolumeCommand` | `emanation.setVolume(volume)` | emanation.cpp | N/A | Set audio volume (0-255) |
| `SET_COLOR` | `SetColorCommand` | `emanation.setColor(r, g, b)` | emanation.cpp | N/A | Set LED RGB color |
| `SET_PATTERN` | `SetPatternCommand` | `emanation.setPattern(pattern)` | emanation.cpp | `p` | Set LED pattern |
| `SET_BRIGHTNESS` | `SetBrightnessCommand` | `emanation.setBrightness(brightness)` | emanation.cpp | N/A | Set LED brightness (0-255) |
| `START_BREATH_SYNC` | `StartBreathSyncCommand` | `emanation.startBreathSync(pattern)` | emanation.cpp | N/A | Start breath synchronization |
| `STOP_BREATH_SYNC` | `StopBreathSyncCommand` | `emanation.stopBreathSync()` | emanation.cpp | N/A | Stop breath synchronization |
| `SET_BINAURAL` | `SetBinauralCommand` | `emanation.setBinauralBeat(config)` | emanation.cpp | N/A | Configure binaural beats |
| `STOP_EMANATION` | `StopEmanationCommand` | `emanation.stop()` | emanation.cpp | N/A | Stop all audio/visual output |
| `LOAD_SIGIL` | `LoadSigilCommand` | `sigilROM.loadSigil(index)`, `emanation.applySigil()` | sigil_rom.cpp, emanation.cpp | N/A | Load sigil parameters |

**SET_FREQUENCY Implementation**:
```cpp
// emanation.cpp
void Emanation::setFrequency(uint16_t frequency) {
    // Set DAC output frequency
    // Frequency range: 20-2000 Hz (Solfeggio: 174-963)
}
```

**SET_PATTERN Implementation**:
```cpp
// emanation.cpp
void Emanation::setPattern(LedPattern pattern) {
    // pattern: SOLID, BREATHE, PULSE, WAVE, SPIRAL, INTERFERENCE, SIGIL
}

// main.cpp, line ~350
case 'p':
    cycleEmanationPattern();  // Cycles through patterns
    break;
```

**LED Pattern Types**:
```cpp
enum class LedPattern : uint8_t {
    SOLID = 0,
    BREATHE = 1,
    PULSE = 2,
    WAVE = 3,
    SPIRAL = 4,
    INTERFERENCE = 5,
    SIGIL = 6
};
```

**LOAD_SIGIL Implementation**:
```cpp
// sigil_rom.cpp
SigilData* SigilROM::loadSigil(uint8_t index) {
    // Read sigil from EEPROM (0-120)
    // Returns frequency, breath pattern, APL operators
}

// emanation.cpp
void Emanation::applySigil(const SigilData& sigil) {
    setFrequency(sigil.frequency);
    setPattern(LedPattern::SIGIL);
    // Apply sigil-specific visual pattern
}
```

### KURAMOTO Commands

| App Command | TypeScript Type | Firmware Function | C++ File | Serial Key | Description |
|-------------|----------------|-------------------|----------|------------|-------------|
| `SET_COUPLING` | `SetCouplingCommand` | `kuramoto.setCoupling(K)` | kuramoto_stabilizer.cpp | `+` / `-` | Set coupling strength K [0, 1] |
| `SET_REFERENCE_FREQ` | `SetReferenceFreqCommand` | `kuramoto.setReferenceFrequency(freq)` | kuramoto_stabilizer.cpp | N/A | Set reference frequency (Hz) |
| `RESET_KURAMOTO` | `ResetKuramotoCommand` | `kuramoto.reset()` | kuramoto_stabilizer.cpp | N/A | Reset oscillator network |
| `SET_TRIAD_THRESHOLDS` | `SetTriadThresholdsCommand` | `triadFSM.setThresholds(high, low)` | triad_fsm.cpp | N/A | Set TRIAD thresholds |
| `FORCE_TRIAD_UNLOCK` | `ForceTriadUnlockCommand` | `triadFSM.forceUnlock()` | triad_fsm.cpp | `t` | Force TRIAD unlock (testing) |

**SET_COUPLING Implementation**:
```cpp
// kuramoto_stabilizer.cpp
void KuramotoStabilizer::setCoupling(float K) {
    // K = coupling strength [0, 1]
    // Affects synchronization dynamics: dθᵢ/dt = ωᵢ + (K/N) Σⱼ sin(θⱼ - θᵢ)
}

// main.cpp, line ~354-370
case '+':  // Increase coupling
    float K = kuramoto.getCoupling() + 0.05f;
    if (K > 1.0f) K = 1.0f;
    kuramoto.setCoupling(K);
    break;

case '-':  // Decrease coupling
    float K = kuramoto.getCoupling() - 0.05f;
    if (K < 0.1f) K = 0.1f;
    kuramoto.setCoupling(K);
    break;
```

**SET_TRIAD_THRESHOLDS Implementation**:
```cpp
// triad_fsm.cpp
void TriadFSM::setThresholds(float high, float low) {
    // high: rising edge threshold (default 0.85)
    // low: re-arm threshold (default 0.82)
    // Must maintain hysteresis: high > low
}
```

**FORCE_TRIAD_UNLOCK Implementation**:
```cpp
// triad_fsm.cpp
void TriadFSM::forceUnlock() {
    // Testing only - bypasses 3-crossing requirement
}

// main.cpp, line ~372
case 't':
    triadFSM.forceUnlock();
    break;
```

### DEBUG Commands

| App Command | TypeScript Type | Firmware Function | C++ File | Serial Key | Description |
|-------------|----------------|-------------------|----------|------------|-------------|
| `READ_SIGIL` | `ReadSigilCommand` | `sigilROM.readSigil(index)` | sigil_rom.cpp | N/A | Read specific sigil from EEPROM |
| `LIST_SIGILS` | `ListSigilsCommand` | `listSigils()` | main.cpp | `l` | List sigils (range) |
| `GET_SENSOR` | `GetSensorCommand` | `hexGrid.getSensor(index)` | hex_grid.cpp | N/A | Get individual sensor reading |
| `DISPLAY_PATTERN` | `DisplayPatternCommand` | `photonic.displayPattern(z, phase, kappa)` | photonic_capture.cpp | N/A | Display photonic pattern |

**LIST_SIGILS Implementation**:
```cpp
// main.cpp
void listSigils() {
    // Iterate through EEPROM
    // Print sigil codes, frequencies, patterns
    // Can specify start index and count
}
```

**GET_SENSOR Implementation**:
```cpp
// hex_grid.cpp
float HexGrid::getSensor(uint8_t index) {
    // index: 0-18 (19 sensors)
    // Returns normalized reading [0, 1]
}
```

## Command Message Format

### WebSocket (JSON)

```json
{
  "type": "COMMAND",
  "version": "1.0.0",
  "timestamp": 1702900000000,
  "payload": {
    "command": "SET_FREQUENCY",
    "category": "EMANATION",
    "payload": {
      "frequency": 528
    },
    "timestamp": 1702900000000,
    "requestId": "req-abc123"
  }
}
```

### BLE (Binary)

Binary format (space-constrained):

```
Header (4 bytes):
  [0]: Message type (0x01 = COMMAND)
  [1-2]: Payload length (little-endian)
  [3]: Flags

Payload: Command-specific binary encoding
```

## Response Format

### Success Response

```json
{
  "type": "COMMAND_RESPONSE",
  "version": "1.0.0",
  "timestamp": 1702900000100,
  "payload": {
    "requestId": "req-abc123",
    "command": "SET_FREQUENCY",
    "status": "OK",
    "data": {
      "frequency": 528,
      "waveform": "SINE"
    },
    "timestamp": 1702900000100
  }
}
```

### Error Response

```json
{
  "type": "COMMAND_RESPONSE",
  "version": "1.0.0",
  "timestamp": 1702900000100,
  "payload": {
    "requestId": "req-abc123",
    "command": "SET_FREQUENCY",
    "status": "ERROR",
    "error": "Frequency out of range (20-2000 Hz)",
    "timestamp": 1702900000100
  }
}
```

## Implementation Notes

### Current Serial Commands (Debug Interface)

The firmware currently implements these single-character serial commands:

| Key | Command | Equivalent App Command |
|-----|---------|------------------------|
| `r` | Reset | `RESET` |
| `s` | Status | `STATUS` |
| `p` | Pattern cycle | `SET_PATTERN` (cycles) |
| `+` | Increase coupling | `SET_COUPLING` (+0.05) |
| `-` | Decrease coupling | `SET_COUPLING` (-0.05) |
| `t` | Force TRIAD unlock | `FORCE_TRIAD_UNLOCK` |
| `l` | List sigils | `LIST_SIGILS` |
| `?` | Help | N/A |

### WebSocket/BLE Command Handler (To Be Implemented)

The full protocol handler should:

1. **Parse incoming messages** (JSON or binary)
2. **Validate command type and payload**
3. **Dispatch to appropriate module function**
4. **Collect response data**
5. **Send response message**

Example implementation:

```cpp
// Protocol handler pseudocode
void handleCommand(const CommandMessage& msg) {
    switch (msg.command) {
        case "SET_FREQUENCY":
            emanation.setFrequency(msg.payload.frequency);
            sendResponse(msg.requestId, "OK", { frequency: msg.payload.frequency });
            break;

        case "CALIBRATE":
            hexGrid.calibrate(msg.payload.samples);
            sendResponse(msg.requestId, "OK", {});
            break;

        // ... etc
    }
}
```

### Module Function Signatures

#### HexGrid (hex_grid.cpp)

```cpp
void calibrate(uint16_t samples);
void setThreshold(float threshold);
void setSmoothingAlpha(float alpha);
float getSensor(uint8_t index);
HexFieldState readField();
```

#### PhaseEngine (phase_engine.cpp)

```cpp
void update(const HexFieldState& field);
PhaseState getState() const;
uint16_t getCurrentFrequency() const;
```

#### TriadFSM (triad_fsm.cpp)

```cpp
void reset();
void update(float coherence);
void setThresholds(float high, float low);
void forceUnlock();
TriadStatus getStatus() const;
```

#### KFormation (k_formation.cpp)

```cpp
void update(const HexFieldState& field);
void resetStats();
KFormationMetrics getMetrics() const;
KFormationStatus getStatus() const;
```

#### Emanation (emanation.cpp)

```cpp
void setOutputMode(uint8_t mode);
void setFrequency(uint16_t frequency);
void setWaveform(Waveform waveform);
void setVolume(uint8_t volume);
void setColor(uint8_t r, uint8_t g, uint8_t b);
void setPattern(LedPattern pattern);
void setBrightness(uint8_t brightness);
void startBreathSync(const BreathPattern& pattern);
void stopBreathSync();
void setBinauralBeat(const BinauralConfig& config);
void stop();
void applySigil(const SigilData& sigil);
```

#### KuramotoStabilizer (kuramoto_stabilizer.cpp)

```cpp
void setCoupling(float K);
void setReferenceFrequency(float freq);
void reset();
KuramotoState getState() const;
StabilizationStatus getStatus() const;
float getCoupling() const;
```

#### SigilROM (sigil_rom.cpp)

```cpp
SigilData* loadSigil(uint8_t index);
SigilData* readSigil(uint8_t index);
void initializeDefaults();
bool isInitialized();
```

## Command Validation

### Frequency Range

- **Solfeggio**: 174, 285, 396, 417, 528, 639, 741, 852, 963 Hz
- **General**: 20-2000 Hz (typical audio range)
- **Binaural beats**: 1-40 Hz (carrier frequency separate)

### Volume/Brightness Range

- **Range**: 0-255 (8-bit)
- **Default**: 128 (50%)

### Coupling Strength (K)

- **Range**: 0.0-1.0
- **Default**: 0.5 (K_CRITICAL)
- **Synchronization**: Typically occurs K > 0.3

### Thresholds

- **Sensor threshold**: 0.0-1.0 (normalized)
- **TRIAD high**: 0.80-0.90 (must be > low)
- **TRIAD low**: 0.75-0.85 (must be < high)
- **Hysteresis required**: high - low ≥ 0.02

## Error Codes

Commands can fail with these error codes (defined in `protocol.ts`):

| Code | Description |
|------|-------------|
| `E_UNKNOWN_COMMAND` | Command not recognized |
| `E_COMMAND_FAILED` | Execution failed |
| `E_DEVICE_BUSY` | Device processing another command |
| `E_INVALID_MESSAGE` | Malformed payload |
| `E_TIMEOUT` | Command timed out |

## Testing Commands

### Basic Test Sequence

1. **Connect**: Establish WebSocket or BLE connection
2. **Status**: Send `STATUS` to verify connection
3. **Calibrate**: Send `CALIBRATE` with samples=50
4. **Set frequency**: Send `SET_FREQUENCY` with frequency=528
5. **Set pattern**: Send `SET_PATTERN` with pattern=BREATHE
6. **Verify**: Check state updates show correct frequency and pattern

### TRIAD Test Sequence

1. **Reset**: Send `RESET` to clear state
2. **Set thresholds**: Send `SET_TRIAD_THRESHOLDS` with high=0.85, low=0.82
3. **Monitor**: Watch for TRIAD_UNLOCK event after 3 crossings
4. **Or force**: Send `FORCE_TRIAD_UNLOCK` for immediate unlock

### K-Formation Test

1. **Unlock TRIAD**: Must be unlocked first
2. **Monitor metrics**: Watch κ, η, R values in state updates
3. **Wait for event**: K_FORMATION_ACHIEVED when all three conditions met

## See Also

- [TypeScript Command Definitions](../WishBed_App_TDD_v2/contracts/interfaces/hardware/ucf-commands.ts)
- [Protocol Specification](../WishBed_App_TDD_v2/contracts/interfaces/hardware/protocol.ts)
- [Constants Reference](./CONSTANTS.md)
- [Integration Guide](./INTEGRATION.md)
- [Hardware README](../unified-consciousness-hardware/README.md)
