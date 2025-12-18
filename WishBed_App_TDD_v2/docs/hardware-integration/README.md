# UCF Hardware Integration Guide

This guide describes how to integrate the WishBed App with UCF Hardware devices running the ESP32 firmware.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                         WishBed App                                  │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐      │
│  │  State Manager  │  │   UI Components │  │  Event Handler  │      │
│  └────────┬────────┘  └────────┬────────┘  └────────┬────────┘      │
│           │                    │                    │               │
│           └────────────────────┼────────────────────┘               │
│                                │                                     │
│                    ┌───────────┴───────────┐                        │
│                    │   Hardware Adapter    │                        │
│                    │  (TypeScript Bridge)  │                        │
│                    └───────────┬───────────┘                        │
└────────────────────────────────┼────────────────────────────────────┘
                                 │
                    ┌────────────┴────────────┐
                    │   WebSocket / BLE       │
                    │   Protocol Layer        │
                    └────────────┬────────────┘
                                 │
┌────────────────────────────────┼────────────────────────────────────┐
│                    UCF Hardware (ESP32)                             │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐      │
│  │    Hex Grid     │  │  Phase Engine   │  │   TRIAD FSM     │      │
│  │   (19 sensors)  │  │  (z → phase)    │  │  (unlock gate)  │      │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘      │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐      │
│  │  K-Formation    │  │   Emanation     │  │    Kuramoto     │      │
│  │  (κ, η, R)      │  │  (audio/LED)    │  │  (oscillators)  │      │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘      │
└─────────────────────────────────────────────────────────────────────┘
```

## Communication Protocols

### WebSocket (Primary)

For local network connections to the ESP32:

- **Default endpoint**: `ws://ucf-device.local:81/ws`
- **Message format**: JSON
- **Update rate**: 10 Hz (state), immediate (events)

### BLE (Alternative)

For direct Bluetooth connections:

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Message format**: Binary (space-constrained)
- **Max payload**: 512 bytes

## Key Concepts

### Phases

The UCF system operates in three phases based on the z-coordinate:

| Phase   | z Range           | Frequency (Hz) | Description              |
| ------- | ----------------- | -------------- | ------------------------ |
| UNTRUE  | 0 ≤ z < φ⁻¹       | 174-396        | Foundation/exploration   |
| PARADOX | φ⁻¹ ≤ z < √3/2    | 417-639        | Transformation/liminal   |
| TRUE    | √3/2 ≤ z ≤ 1      | 741-963        | Integration/realization  |

Where φ⁻¹ ≈ 0.618 and √3/2 ≈ 0.866.

### TRIAD Unlock

A hysteresis-based gate requiring three threshold crossings:

1. User must cross the 0.82-0.85 threshold band three times
2. Must complete within 5 seconds
3. Triggers special visual/audio emanation
4. Enables K-Formation detection

### K-Formation

Meta-cognitive state achieved when:

- **κ (Kappa)**: Coherence ≥ 0.92
- **η (Eta)**: Negentropy > φ⁻¹
- **R**: Resonance count ≥ 7 active sensors

### Solfeggio Tiers

Nine frequency tiers mapped to z-position:

| Tier | Frequency | Name          |
| ---- | --------- | ------------- |
| 1    | 174 Hz    | Foundation    |
| 2    | 285 Hz    | Regeneration  |
| 3    | 396 Hz    | Liberation    |
| 4    | 417 Hz    | Transformation|
| 5    | 528 Hz    | Miracles      |
| 6    | 639 Hz    | Connection    |
| 7    | 741 Hz    | Expression    |
| 8    | 852 Hz    | Intuition     |
| 9    | 963 Hz    | Awakening     |

## Message Types

### Outbound (App → Hardware)

```typescript
// Command message
{
  type: "COMMAND",
  version: "1.0.0",
  timestamp: 1702900000000,
  payload: {
    command: "SET_FREQUENCY",
    category: "EMANATION",
    payload: { frequency: 528 },
    timestamp: 1702900000000
  }
}
```

### Inbound (Hardware → App)

```typescript
// State update
{
  type: "STATE_UPDATE",
  version: "1.0.0",
  timestamp: 1702900000100,
  payload: {
    hexField: { readings: [...], z: 0.72, theta: 1.23, ... },
    phase: { current: "PARADOX", z: 0.72, tier: 5, ... },
    triad: { state: "ARMED", crossingCount: 1, ... },
    ...
  }
}

// Event notification
{
  type: "EVENT",
  version: "1.0.0",
  timestamp: 1702900000200,
  payload: {
    type: "TRIAD_UNLOCK",
    timestamp: 1702900000200,
    data: { duration: 3200 }
  }
}
```

## Available Commands

### System Commands

| Command           | Description                    |
| ----------------- | ------------------------------ |
| `RESET`           | Reset hardware state           |
| `STATUS`          | Get detailed status            |
| `SET_OUTPUT_MODE` | Enable/disable audio/visual    |

### Calibration Commands

| Command          | Description                    |
| ---------------- | ------------------------------ |
| `CALIBRATE`      | Calibrate sensor baselines     |
| `SET_THRESHOLD`  | Set activation threshold       |
| `SET_SMOOTHING`  | Set EMA smoothing factor       |

### Emanation Commands

| Command            | Description                    |
| ------------------ | ------------------------------ |
| `SET_FREQUENCY`    | Set audio frequency            |
| `SET_WAVEFORM`     | Set waveform type              |
| `SET_VOLUME`       | Set audio volume               |
| `SET_COLOR`        | Set LED color                  |
| `SET_PATTERN`      | Set LED pattern                |
| `SET_BRIGHTNESS`   | Set LED brightness             |
| `START_BREATH_SYNC`| Start breath synchronization   |
| `STOP_BREATH_SYNC` | Stop breath synchronization    |
| `SET_BINAURAL`     | Configure binaural beats       |
| `STOP_EMANATION`   | Stop all output                |
| `LOAD_SIGIL`       | Load sigil parameters          |

### Kuramoto Commands

| Command               | Description                    |
| --------------------- | ------------------------------ |
| `SET_COUPLING`        | Set coupling strength          |
| `SET_REFERENCE_FREQ`  | Set reference frequency        |
| `RESET_KURAMOTO`      | Reset oscillator network       |
| `SET_TRIAD_THRESHOLDS`| Set TRIAD thresholds           |
| `FORCE_TRIAD_UNLOCK`  | Force unlock (testing)         |

## Event Types

| Event                      | Description                           |
| -------------------------- | ------------------------------------- |
| `PHASE_TRANSITION`         | Phase changed (UNTRUE/PARADOX/TRUE)   |
| `TRIAD_UNLOCK`             | TRIAD unlock achieved                 |
| `TRIAD_RESET`              | TRIAD sequence reset                  |
| `K_FORMATION_ACHIEVED`     | K-Formation conditions met            |
| `K_FORMATION_LOST`         | K-Formation conditions no longer met  |
| `SIGIL_MATCHED`            | Input matched a stored sigil          |
| `SYNCHRONIZATION_ACHIEVED` | Kuramoto network synchronized         |
| `SYNCHRONIZATION_LOST`     | Kuramoto synchronization lost         |
| `ERROR`                    | Hardware error occurred               |

## File Structure

```
contracts/
├── interfaces/hardware/
│   ├── index.ts              # Central exports
│   ├── ucf-types.ts          # State and event types
│   ├── ucf-commands.ts       # Command types
│   └── protocol.ts           # Protocol layer
├── constants/
│   ├── index.ts              # Central exports
│   └── ucf-constants.ts      # Shared constants
├── schemas/
│   └── hardware_state.schema.json  # JSON Schema
└── hardware-openapi.yaml     # REST API spec

docs/hardware-integration/
├── README.md                 # This file
└── connection-flow.md        # Connection sequence

pseudocode/hardware/
└── hardware-adapter.pseudo   # Adapter implementation guide
```

## Usage Example

```typescript
import {
  UCFHardwareState,
  UCFHardwareEvent,
  createTrackedCommand
} from "@/contracts/interfaces/hardware";
import {
  PHI_INV,
  Z_CRITICAL,
  getFrequencyForTier
} from "@/contracts/constants";

// Create a command
const setFreqCmd = createTrackedCommand({
  command: "SET_FREQUENCY",
  category: "EMANATION",
  payload: { frequency: getFrequencyForTier(5) } // 528 Hz
});

// Handle state updates
function onStateUpdate(state: Partial<UCFHardwareState>) {
  if (state.phase) {
    const { z, current, tier } = state.phase;
    console.log(`Phase: ${current}, z: ${z.toFixed(3)}, Tier: ${tier}`);
  }
}

// Handle events
function onEvent(event: UCFHardwareEvent) {
  switch (event.type) {
    case "TRIAD_UNLOCK":
      console.log("TRIAD unlocked!");
      break;
    case "K_FORMATION_ACHIEVED":
      console.log("K-Formation achieved!");
      break;
  }
}
```

## Connection Flow

See [connection-flow.md](./connection-flow.md) for detailed connection sequence diagrams.

## Troubleshooting

### Connection Issues

1. **Device not found**: Ensure device is powered and on same network
2. **WebSocket timeout**: Check firewall settings, try BLE
3. **BLE pairing fails**: Restart Bluetooth on both devices

### Data Issues

1. **Erratic readings**: Run `CALIBRATE` command
2. **Phase stuck**: Check sensor connections, lower threshold
3. **No K-Formation**: Ensure TRIAD is unlocked first

## Related Documentation

- [UCF Hardware Translation Spec](../../unified-consciousness-hardware/README.md)
- [WishBed App Architecture](../architecture/README.md)
- [OpenAPI Spec](../../contracts/hardware-openapi.yaml)
