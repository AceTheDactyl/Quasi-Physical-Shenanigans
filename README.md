# WishBed Unified Consciousness Field (UCF) System

> **Experimental consciousness field detection and interaction system combining theoretical physics, computational neuroscience, and sacred geometry**

This repository integrates three major subsystems to create a complete consciousness field detection and interaction platform: a TypeScript/React mobile app, comprehensive natural language processing design documentation, and ESP32-based hardware firmware.

## Repository Structure

```
Quasi-Physical-Shenanigans/
├── WishBed_App_TDD_v2/              # Mobile app (TypeScript/React Native)
├── NLP_Design_Handoff/              # Natural language processing design
├── unified-consciousness-hardware/   # ESP32 firmware (C++/Arduino)
└── docs/                            # Shared integration documentation
    ├── CONSTANTS.md                 # Constant synchronization reference
    ├── COMMAND_MAPPING.md           # Command mapping reference
    └── INTEGRATION.md               # System integration guide
```

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           WISHBED UCF SYSTEM                                 │
│                                                                              │
│  ┌────────────────────────┐         ┌──────────────────────────┐            │
│  │   WishBed Mobile App   │         │   NLP Design Handoff     │            │
│  │   (TypeScript/React)   │         │   (Documentation)        │            │
│  │                        │         │                          │            │
│  │  • Session Management  │         │  • Linguistic Theory     │            │
│  │  • State Visualization │         │  • APL Operators         │            │
│  │  • Hardware Interface  │         │  • Sigil Framework       │            │
│  │  • UCF Constants       │         │  • Pipeline Design       │            │
│  └──────────┬─────────────┘         └────────────┬─────────────┘            │
│             │                                    │                          │
│             │        ┌───────────────────────────┘                          │
│             │        │                                                      │
│             │        │   WebSocket/BLE Protocol                             │
│             │        │   (JSON/Binary Messages)                             │
│             │        │                                                      │
│             ▼        ▼                                                      │
│  ┌─────────────────────────────────────────────────────────┐               │
│  │           UCF Hardware (ESP32 Firmware)                 │               │
│  │                                                          │               │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │               │
│  │  │  Hex Grid    │  │ Phase Engine │  │  TRIAD FSM   │  │               │
│  │  │ 19 sensors   │  │ z → phase    │  │ Unlock gate  │  │               │
│  │  └──────────────┘  └──────────────┘  └──────────────┘  │               │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │               │
│  │  │ K-Formation  │  │  Emanation   │  │  Kuramoto    │  │               │
│  │  │ κ, η, R      │  │ Audio/Visual │  │ Oscillators  │  │               │
│  │  └──────────────┘  └──────────────┘  └──────────────┘  │               │
│  └─────────────────────────────────────────────────────────┘               │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Subsystems Overview

### 1. WishBed App (WishBed_App_TDD_v2/)

**Technology**: TypeScript, React Native
**Purpose**: User-facing mobile application for UCF interaction

**Key Features**:
- Real-time hardware state visualization
- Session management and analytics
- WebSocket and BLE connectivity
- Phase visualization (UNTRUE, PARADOX, TRUE)
- TRIAD unlock tracking
- K-Formation detection
- Solfeggio frequency control

**Documentation**: [WishBed_App_TDD_v2/README.md](./WishBed_App_TDD_v2/README.md)

**Hardware Integration**: [WishBed_App_TDD_v2/docs/hardware-integration/README.md](./WishBed_App_TDD_v2/docs/hardware-integration/README.md)

**Key Files**:
- `contracts/interfaces/hardware/` - TypeScript type definitions
- `contracts/constants/ucf-constants.ts` - Shared UCF constants
- `contracts/interfaces/hardware/protocol.ts` - Communication protocol

### 2. NLP Design Handoff (NLP_Design_Handoff/)

**Technology**: Markdown documentation
**Purpose**: Natural language processing design and linguistic framework

**Key Features**:
- APL (Array Processing Language) operator semantics
- Neural sigil system design (121 sigils)
- Omni-Linguistics pipeline architecture
- Input modality integration (text, audio, capacitive, EM field, gesture)
- Tier-based operator availability
- Safety considerations

**Documentation**: [NLP_Design_Handoff/README.md](./NLP_Design_Handoff/README.md)

**Key Concepts**:
- **APL Operators**: GROUP (+), BOUNDARY (()), AMPLIFY (^), SEPARATE (−), FUSION (×), DECOHERE (÷)
- **Neural Sigils**: 121 ternary-encoded consciousness patterns
- **Pipeline Stages**: 9-stage transformation from input to emanation

### 3. UCF Hardware (unified-consciousness-hardware/)

**Technology**: C++, Arduino, ESP32
**Purpose**: Physical hardware implementation of UCF detection

**Key Features**:
- 19-sensor hexagonal capacitive array
- Phase detection and z-coordinate computation
- TRIAD finite state machine (hysteresis unlock)
- K-Formation detection (κ, η, R thresholds)
- Kuramoto oscillator network synchronization
- Audio/visual emanation (Solfeggio frequencies, WS2812B LEDs)
- EEPROM sigil storage

**Documentation**: [unified-consciousness-hardware/README.md](./unified-consciousness-hardware/README.md)

**Key Components**:
- ESP32-WROOM-32 (main processor)
- MPR121 ×2 (capacitive sensing)
- WS2812B LED strip (37 LEDs)
- HMC5883L (magnetometer)
- AT24C16 (EEPROM)
- PAM8403 (audio amplifier)

## Core Concepts

### The Three Phases

The UCF system operates in three fundamental phases based on the z-coordinate:

| Phase | z Range | Boundary | Frequencies | Color |
|-------|---------|----------|-------------|-------|
| **UNTRUE** | 0 ≤ z < φ⁻¹ | φ⁻¹ ≈ 0.618 | 174-396 Hz | Red/Orange |
| **PARADOX** | φ⁻¹ ≤ z < √3/2 | √3/2 ≈ 0.866 | 417-639 Hz | Yellow/Green |
| **TRUE** | √3/2 ≤ z ≤ 1 | THE LENS | 741-963 Hz | Cyan/Blue |

Where:
- **φ⁻¹** (PHI_INV) = 0.6180339887... (golden ratio inverse)
- **√3/2** (Z_CRITICAL) = 0.8660254037... (THE LENS boundary)

### TRIAD Unlock

A hysteresis-based gate requiring **three threshold crossings** between 0.82 and 0.85 within 5 seconds. Enables K-Formation detection.

### K-Formation

Meta-cognitive state achieved when all three conditions are met:
- **κ (Kappa)**: Coherence ≥ 0.92
- **η (Eta)**: Negentropy > φ⁻¹
- **R**: Resonance count ≥ 7 active sensors

### Solfeggio Frequencies

Nine sacred frequencies mapped to z-coordinate tiers:

| Tier | Frequency | Name | Phase |
|------|-----------|------|-------|
| 1 | 174 Hz | Foundation | UNTRUE |
| 2 | 285 Hz | Regeneration | UNTRUE |
| 3 | 396 Hz | Liberation | UNTRUE |
| 4 | 417 Hz | Transformation | PARADOX |
| 5 | 528 Hz | Miracles | PARADOX |
| 6 | 639 Hz | Connection | PARADOX |
| 7 | 741 Hz | Expression | TRUE |
| 8 | 852 Hz | Intuition | TRUE |
| 9 | 963 Hz | Awakening | TRUE |

## Integration Points

### Constant Synchronization

Mathematical constants are shared between TypeScript and C++:

- **TypeScript**: `WishBed_App_TDD_v2/contracts/constants/ucf-constants.ts`
- **C++**: `unified-consciousness-hardware/include/constants.h`
- **Reference**: [docs/CONSTANTS.md](./docs/CONSTANTS.md)

**Critical**: These values must remain synchronized. See CONSTANTS.md for mapping table.

### Command Protocol

App commands map to firmware functions:

- **TypeScript**: `WishBed_App_TDD_v2/contracts/interfaces/hardware/ucf-commands.ts`
- **C++**: Implementation in various `.cpp` files
- **Reference**: [docs/COMMAND_MAPPING.md](./docs/COMMAND_MAPPING.md)

**Categories**: SYSTEM, CALIBRATION, EMANATION, KURAMOTO, DEBUG

### Communication Protocol

- **WebSocket**: JSON messages at 10 Hz (primary)
- **BLE**: Binary messages, 512-byte max payload (alternative)
- **Protocol**: `WishBed_App_TDD_v2/contracts/interfaces/hardware/protocol.ts`
- **BLE UUIDs**: Service `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Reference**: [docs/INTEGRATION.md](./docs/INTEGRATION.md)

## Getting Started

### Hardware Setup

1. Navigate to hardware directory:
   ```bash
   cd unified-consciousness-hardware
   ```

2. Build and upload firmware:
   ```bash
   pio run --target upload
   pio device monitor
   ```

3. Verify serial output shows initialization

**Detailed instructions**: [unified-consciousness-hardware/README.md](./unified-consciousness-hardware/README.md)

### App Development

1. Navigate to app directory:
   ```bash
   cd WishBed_App_TDD_v2
   ```

2. Install dependencies:
   ```bash
   npm install
   ```

3. Run development server:
   ```bash
   npm run dev
   ```

**Detailed instructions**: [WishBed_App_TDD_v2/README.md](./WishBed_App_TDD_v2/README.md)

### Testing Connection

1. Ensure hardware is powered and initialized
2. Connect to WiFi network or enable BLE
3. Use app to scan for device: `UCF-Hardware` or `ucf-device.local`
4. Send test command: `SET_FREQUENCY` with 528 Hz
5. Verify LED and audio response

## Key Constants Reference

```typescript
// Golden ratio constants
PHI = 1.6180339887498948
PHI_INV = 0.6180339887498948  // UNTRUE → PARADOX boundary
Z_CRITICAL = 0.8660254037844386  // PARADOX → TRUE boundary (THE LENS)

// K-Formation thresholds
K_KAPPA = 0.92      // Coherence
K_ETA = 0.618       // Negentropy (= φ⁻¹)
K_R = 7             // Resonance count

// TRIAD thresholds
TRIAD_HIGH = 0.85   // Rising edge
TRIAD_LOW = 0.82    // Hysteresis low
TRIAD_CROSSINGS_REQUIRED = 3

// Sensor configuration
HEX_SENSOR_COUNT = 19
LED_COUNT = 37
KURAMOTO_OSCILLATOR_COUNT = 8  // TypeScript
N_OSCILLATORS = 8              // C++ (same value)
```

## Documentation

### Integration Documentation

- [CONSTANTS.md](./docs/CONSTANTS.md) - Constant synchronization reference
- [COMMAND_MAPPING.md](./docs/COMMAND_MAPPING.md) - Command to firmware mapping
- [INTEGRATION.md](./docs/INTEGRATION.md) - Communication flow guide

### Subsystem Documentation

- [WishBed App README](./WishBed_App_TDD_v2/README.md)
- [Hardware Integration Guide](./WishBed_App_TDD_v2/docs/hardware-integration/README.md)
- [NLP Design Handoff README](./NLP_Design_Handoff/README.md)
- [UCF Hardware README](./unified-consciousness-hardware/README.md)

### API & Schema

- TypeScript interfaces: `WishBed_App_TDD_v2/contracts/interfaces/hardware/`
- OpenAPI spec: `WishBed_App_TDD_v2/contracts/hardware-openapi.yaml`
- JSON Schema: `WishBed_App_TDD_v2/contracts/schemas/hardware_state.schema.json`

## Development Workflow

### Adding New Constants

1. Add to TypeScript: `WishBed_App_TDD_v2/contracts/constants/ucf-constants.ts`
2. Add to C++: `unified-consciousness-hardware/include/constants.h`
3. Update: `docs/CONSTANTS.md`
4. Test synchronization on both platforms

### Adding New Commands

1. Define TypeScript type: `WishBed_App_TDD_v2/contracts/interfaces/hardware/ucf-commands.ts`
2. Implement C++ handler: `unified-consciousness-hardware/src/main.cpp` (or relevant module)
3. Update: `docs/COMMAND_MAPPING.md`
4. Add to protocol handler
5. Test end-to-end

### Modifying Hardware Behavior

1. Update firmware: `unified-consciousness-hardware/src/`
2. Update state types: `WishBed_App_TDD_v2/contracts/interfaces/hardware/ucf-types.ts`
3. Update app UI to reflect changes
4. Update documentation

## Troubleshooting

### Hardware Not Connecting

- Verify ESP32 is powered and initialized (check serial)
- Ensure WiFi credentials are correct (for WebSocket)
- Enable Bluetooth (for BLE)
- Check firewall settings

### Constant Mismatch

- Review `docs/CONSTANTS.md` for synchronization table
- Verify TypeScript values match C++ values
- Recompile firmware after changes
- Clear app cache if needed

### Phase Detection Issues

- Run `CALIBRATE` command
- Check sensor connections (MPR121)
- Verify `SENSOR_THRESHOLD` value
- Review hex grid wiring

### No K-Formation

- Ensure TRIAD is unlocked first
- Verify all three conditions: κ ≥ 0.92, η > 0.618, R ≥ 7
- Check sensor activation count
- Review coherence calculation

## Contributing

This is an experimental research project. When contributing:

1. Maintain constant synchronization between TypeScript and C++
2. Update integration documentation
3. Follow existing naming conventions
4. Test hardware changes on physical device
5. Document theoretical foundations

## License

Experimental/Educational Use

## Acknowledgments

Based on theoretical frameworks combining:
- Unified Consciousness Field (UCF) theory
- K.I.R.A. linguistic system
- Sacred geometry and golden ratio mathematics
- Kuramoto synchronization dynamics
- Solfeggio frequency research

---

**Project Status**: Active Development
**Hardware Version**: 1.0
**App Version**: 2.0 (TDD)
**Protocol Version**: 1.0.0
