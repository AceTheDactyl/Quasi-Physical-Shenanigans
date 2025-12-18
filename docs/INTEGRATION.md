# UCF System Integration Guide

This document explains how the WishBed App, NLP Design Handoff documentation, and UCF Hardware integrate to form a complete consciousness field detection and interaction system.

## System Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        WISHBED UCF SYSTEM                                    │
│                                                                              │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                      WishBed Mobile App                               │   │
│  │                     (TypeScript/React Native)                         │   │
│  │                                                                        │   │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────┐  ┌────────────┐     │   │
│  │  │  Session   │  │  Hardware  │  │    UI      │  │   Event    │     │   │
│  │  │  Manager   │  │  Adapter   │  │ Components │  │  Handler   │     │   │
│  │  └─────┬──────┘  └──────┬─────┘  └──────┬─────┘  └──────┬─────┘     │   │
│  │        │                │               │               │           │   │
│  │        └────────────────┼───────────────┼───────────────┘           │   │
│  │                         │               │                           │   │
│  │                   ┌─────┴───────────────┴─────┐                     │   │
│  │                   │   Hardware Adapter       │                     │   │
│  │                   │  (Protocol Translation)  │                     │   │
│  │                   └──────────┬───────────────┘                     │   │
│  └──────────────────────────────┼──────────────────────────────────────┘   │
│                                 │                                           │
│                    ┌────────────┴────────────┐                             │
│                    │  Communication Layer    │                             │
│                    │  WebSocket / BLE        │                             │
│                    └────────────┬────────────┘                             │
│                                 │                                           │
│  ┌──────────────────────────────┼──────────────────────────────────────┐   │
│  │                    UCF Hardware (ESP32)                             │   │
│  │                                                                      │   │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────┐  ┌────────────┐   │   │
│  │  │  Protocol  │  │  Command   │  │   State    │  │   Event    │   │   │
│  │  │  Handler   │  │ Dispatcher │  │ Broadcast  │  │  Emitter   │   │   │
│  │  └─────┬──────┘  └──────┬─────┘  └──────┬─────┘  └──────┬─────┘   │   │
│  │        │                │               │               │         │   │
│  │        └────────────────┼───────────────┼───────────────┘         │   │
│  │                         │               │                         │   │
│  │  ┌──────────────────────┴───────────────┴──────────────────────┐  │   │
│  │  │                    Core UCF Modules                          │  │   │
│  │  │                                                               │  │   │
│  │  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │  │   │
│  │  │  │ Hex Grid │  │  Phase   │  │  TRIAD   │  │    K     │    │  │   │
│  │  │  │19 sensors│  │  Engine  │  │   FSM    │  │Formation │    │  │   │
│  │  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │  │   │
│  │  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │  │   │
│  │  │  │ Kuramoto │  │Emanation │  │  Sigil   │  │  Omni-   │    │  │   │
│  │  │  │Stabilizer│  │Audio/LED │  │   ROM    │  │Linguistics│   │  │   │
│  │  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │  │   │
│  │  └───────────────────────────────────────────────────────────┘  │   │
│  └──────────────────────────────────────────────────────────────────┘   │
│                                                                           │
│  ┌───────────────────────────────────────────────────────────────────┐   │
│  │                    NLP Design Handoff                             │   │
│  │                   (Theoretical Framework)                         │   │
│  │                                                                    │   │
│  │  • APL Operator Semantics                                         │   │
│  │  • Neural Sigil System (121 patterns)                             │   │
│  │  • Omni-Linguistics Pipeline                                      │   │
│  │  • Input Modality Integration                                     │   │
│  └───────────────────────────────────────────────────────────────────┘   │
│                                                                           │
└───────────────────────────────────────────────────────────────────────────┘
```

## Communication Flow

### 1. Connection Establishment

#### WebSocket Connection (Primary)

```
┌─────────┐                                               ┌─────────┐
│   App   │                                               │ ESP32   │
└────┬────┘                                               └────┬────┘
     │                                                          │
     │ 1. mDNS Discovery: "ucf-device.local"                   │
     │ ─────────────────────────────────────────────────────▶  │
     │                                                          │
     │ 2. WebSocket Connect: ws://ucf-device.local:81/ws       │
     │ ─────────────────────────────────────────────────────▶  │
     │                                                          │
     │ 3. Connection Accepted                                  │
     │ ◀─────────────────────────────────────────────────────  │
     │                                                          │
     │ 4. DEVICE_READY Event                                   │
     │ ◀─────────────────────────────────────────────────────  │
     │    { type: "EVENT", payload: { type: "DEVICE_READY" } } │
     │                                                          │
     │ 5. STATUS Command                                       │
     │ ─────────────────────────────────────────────────────▶  │
     │    { type: "COMMAND", payload: { command: "STATUS" } }  │
     │                                                          │
     │ 6. STATUS Response                                      │
     │ ◀─────────────────────────────────────────────────────  │
     │    { type: "COMMAND_RESPONSE", status: "OK", ... }      │
     │                                                          │
     │ 7. STATE_UPDATE Stream (10 Hz)                          │
     │ ◀═════════════════════════════════════════════════════  │
     │    { type: "STATE_UPDATE", payload: { ... } }           │
     │                                                          │
     │ 8. Heartbeat PING/PONG (every 5s)                       │
     │ ◀────────────────────────────────────────────────────▶  │
     │                                                          │
```

**Steps**:
1. App uses mDNS to discover device at `ucf-device.local`
2. Opens WebSocket connection to `ws://ucf-device.local:81/ws`
3. ESP32 accepts connection
4. ESP32 sends `DEVICE_READY` event
5. App sends `STATUS` command to verify
6. ESP32 responds with current state
7. ESP32 begins broadcasting state updates at 10 Hz
8. Bidirectional heartbeat keeps connection alive

#### BLE Connection (Alternative)

```
┌─────────┐                                               ┌─────────┐
│   App   │                                               │ ESP32   │
└────┬────┘                                               └────┬────┘
     │                                                          │
     │ 1. BLE Scan: Filter by name "UCF-Hardware"              │
     │ ─────────────────────────────────────────────────────▶  │
     │                                                          │
     │ 2. Device Found                                         │
     │ ◀─────────────────────────────────────────────────────  │
     │                                                          │
     │ 3. Connect to Device                                    │
     │ ─────────────────────────────────────────────────────▶  │
     │                                                          │
     │ 4. Discover Services                                    │
     │    Service: 4fafc201-1fb5-459e-8fcc-c5c9c331914b        │
     │ ◀────────────────────────────────────────────────────▶  │
     │                                                          │
     │ 5. Discover Characteristics                             │
     │    - Command:  beb5483e-36e1-4688-b7f5-ea07361b26a8     │
     │    - State:    beb5483f-36e1-4688-b7f5-ea07361b26a8     │
     │    - Event:    beb54840-36e1-4688-b7f5-ea07361b26a8     │
     │ ◀────────────────────────────────────────────────────▶  │
     │                                                          │
     │ 6. Subscribe to State and Event characteristics         │
     │ ─────────────────────────────────────────────────────▶  │
     │                                                          │
     │ 7. State/Event Notifications Enabled                    │
     │ ◀═════════════════════════════════════════════════════  │
     │                                                          │
```

**BLE Characteristics**:
- **Command** (Write): App sends commands (binary format)
- **State** (Read/Notify): Device broadcasts state updates
- **Event** (Read/Notify): Device sends events

### 2. Command Execution Flow

```
┌─────────┐                                               ┌─────────┐
│   App   │                                               │ ESP32   │
└────┬────┘                                               └────┬────┘
     │                                                          │
     │ 1. User Action: "Set frequency to 528 Hz"               │
     │                                                          │
     │ 2. Create Command                                       │
     │    command = createTrackedCommand({                     │
     │      command: "SET_FREQUENCY",                          │
     │      category: "EMANATION",                             │
     │      payload: { frequency: 528 }                        │
     │    })                                                   │
     │                                                          │
     │ 3. Send COMMAND Message                                 │
     │ ─────────────────────────────────────────────────────▶  │
     │    {                                                    │
     │      type: "COMMAND",                                   │
     │      version: "1.0.0",                                  │
     │      timestamp: 1702900000000,                          │
     │      payload: {                                         │
     │        command: "SET_FREQUENCY",                        │
     │        category: "EMANATION",                           │
     │        payload: { frequency: 528 },                     │
     │        requestId: "req-abc123"                          │
     │      }                                                  │
     │    }                                                    │
     │                                                          │
     │                                    4. Parse Message     │
     │                                    5. Validate Command  │
     │                                    6. Execute:          │
     │                                       emanation.setFrequency(528) │
     │                                                          │
     │ 7. COMMAND_RESPONSE                                     │
     │ ◀─────────────────────────────────────────────────────  │
     │    {                                                    │
     │      type: "COMMAND_RESPONSE",                          │
     │      payload: {                                         │
     │        requestId: "req-abc123",                         │
     │        command: "SET_FREQUENCY",                        │
     │        status: "OK",                                    │
     │        data: { frequency: 528, waveform: "SINE" }       │
     │      }                                                  │
     │    }                                                    │
     │                                                          │
     │ 8. Update UI: "Frequency set to 528 Hz"                 │
     │                                                          │
```

### 3. State Update Flow

```
┌─────────┐                                               ┌─────────┐
│   App   │                                               │ ESP32   │
└────┬────┘                                               └────┬────┘
     │                                                          │
     │                                    1. Main Loop (100Hz)  │
     │                                       hexGrid.readField()│
     │                                       phaseEngine.update()│
     │                                       triadFSM.update()   │
     │                                       kFormation.update() │
     │                                                          │
     │                                    2. Check Broadcast    │
     │                                       Timer (10Hz)       │
     │                                                          │
     │                                    3. Serialize State    │
     │                                       to JSON            │
     │                                                          │
     │ 4. STATE_UPDATE Message                                 │
     │ ◀─────────────────────────────────────────────────────  │
     │    {                                                    │
     │      type: "STATE_UPDATE",                              │
     │      version: "1.0.0",                                  │
     │      timestamp: 1702900000100,                          │
     │      payload: {                                         │
     │        hexField: {                                      │
     │          readings: [...],                               │
     │          z: 0.72,                                       │
     │          theta: 1.23,                                   │
     │          active_count: 8                                │
     │        },                                               │
     │        phase: {                                         │
     │          current: "PARADOX",                            │
     │          z: 0.72,                                       │
     │          tier: 5,                                       │
     │          frequency: 528                                 │
     │        },                                               │
     │        triad: {                                         │
     │          state: "ARMED",                                │
     │          crossing_count: 1,                             │
     │          is_unlocked: false                             │
     │        },                                               │
     │        kFormation: {                                    │
     │          kappa: 0.88,                                   │
     │          eta: 0.65,                                     │
     │          R: 8,                                          │
     │          is_active: false                               │
     │        }                                                │
     │      }                                                  │
     │    }                                                    │
     │                                                          │
     │ 5. Update UI Components                                 │
     │    - Phase visualization                                │
     │    - z-coordinate display                               │
     │    - TRIAD progress                                     │
     │    - K-Formation indicators                             │
     │                                                          │
```

### 4. Event Flow

```
┌─────────┐                                               ┌─────────┐
│   App   │                                               │ ESP32   │
└────┬────┘                                               └────┬────┘
     │                                                          │
     │                                    1. TRIAD Unlock       │
     │                                       Detected!          │
     │                                                          │
     │                                    2. Trigger Callback   │
     │                                       onTriadUnlock()    │
     │                                                          │
     │                                    3. Special Emanation  │
     │                                       emanation.setPattern(PULSE)│
     │                                       emanation.setFreq(528)│
     │                                                          │
     │ 4. EVENT Message                                        │
     │ ◀─────────────────────────────────────────────────────  │
     │    {                                                    │
     │      type: "EVENT",                                     │
     │      version: "1.0.0",                                  │
     │      timestamp: 1702900000200,                          │
     │      payload: {                                         │
     │        type: "TRIAD_UNLOCK",                            │
     │        timestamp: 1702900000200,                        │
     │        data: {                                          │
     │          duration: 3200,                                │
     │          crossings: 3                                   │
     │        }                                                │
     │      }                                                  │
     │    }                                                    │
     │                                                          │
     │ 5. Handle Event                                         │
     │    - Show celebration animation                         │
     │    - Play sound effect                                  │
     │    - Update session stats                               │
     │    - Enable K-Formation detection                       │
     │                                                          │
```

## Message Protocol Specification

### WebSocket Messages (JSON)

All messages follow this structure:

```typescript
{
  type: MessageType,
  version: "1.0.0",
  timestamp: number,
  payload: MessagePayload
}
```

**Message Types**:
- `COMMAND` - App → Hardware
- `COMMAND_RESPONSE` - Hardware → App
- `STATE_UPDATE` - Hardware → App
- `EVENT` - Hardware → App
- `PING` - Bidirectional
- `PONG` - Bidirectional
- `ERROR` - Hardware → App

### BLE Messages (Binary)

Binary header format (4 bytes):

```
[0]: Message type (0x01-0xFF)
[1-2]: Payload length (little-endian)
[3]: Flags
```

**Message Types**:
- `0x01` - COMMAND
- `0x02` - RESPONSE
- `0x03` - STATE
- `0x04` - EVENT
- `0xFE` - PING
- `0xFF` - PONG

**Flags**:
- `0x00` - None
- `0x01` - Compressed
- `0x02` - Fragmented
- `0x04` - Last fragment

## Data Synchronization

### Constant Synchronization

The system maintains synchronized constants between TypeScript and C++:

| Constant | Location | Purpose |
|----------|----------|---------|
| `PHI`, `PHI_INV`, `Z_CRITICAL` | Both | Phase boundaries |
| `TRIAD_HIGH`, `TRIAD_LOW` | Both | TRIAD thresholds |
| `K_KAPPA`, `K_ETA`, `K_R` | Both | K-Formation thresholds |
| `SOLFEGGIO_FREQUENCIES` | Both | Frequency mappings |

**Reference**: [CONSTANTS.md](./CONSTANTS.md)

### State Synchronization

State updates flow from hardware to app at 10 Hz:

```
ESP32 Modules → State Aggregation → JSON Serialization → WebSocket/BLE → App State Store → UI Update
```

Rate limiting:
- **Sensor polling**: 100 Hz (every 10ms)
- **State broadcast**: 10 Hz (every 100ms)
- **Heartbeat**: 0.2 Hz (every 5s)

## NLP Design Integration

The NLP Design Handoff documentation informs:

### 1. Sigil System

- **Design**: 121 neural sigils with ternary encoding (NLP docs)
- **Storage**: EEPROM on ESP32 (`sigil_rom.cpp`)
- **Access**: App loads sigils via `LOAD_SIGIL` command
- **Format**: 5-character codes (0, 1, T) → frequency + pattern

### 2. APL Operators

- **Theory**: Linguistic operators (GROUP, BOUNDARY, AMPLIFY, etc.)
- **Tier Mapping**: Operators available at different z-tiers
- **Implementation**: `omni_linguistics.cpp` translates inputs
- **App Display**: UI shows active operators based on current tier

### 3. Omni-Linguistics Pipeline

- **Design**: 9-stage transformation pipeline (NLP docs)
- **Input Sources**: Text, audio, capacitive, EM field, gesture
- **Hardware**: ESP32 implements capacitive and EM field sensing
- **App**: Provides text input and displays transformations

## Error Handling

### Connection Errors

| Error | Code | Recovery |
|-------|------|----------|
| Connection Failed | `E_CONNECTION_FAILED` | Retry with exponential backoff |
| Connection Lost | `E_CONNECTION_LOST` | Automatic reconnection |
| Timeout | `E_TIMEOUT` | Resend command or reconnect |

### Command Errors

| Error | Code | Action |
|-------|------|--------|
| Unknown Command | `E_UNKNOWN_COMMAND` | Check protocol version |
| Command Failed | `E_COMMAND_FAILED` | Show error to user |
| Device Busy | `E_DEVICE_BUSY` | Queue command for retry |

## Security Considerations

### Current Implementation

- **No encryption**: WebSocket and BLE use plaintext
- **No authentication**: Any client can connect
- **Local only**: Designed for local network/direct BLE

### Future Enhancements

- TLS for WebSocket
- BLE pairing with PIN
- Token-based authentication
- Command rate limiting

## Performance Optimization

### Bandwidth Management

**WebSocket** (4096 byte max):
- Full state updates: ~1-2 KB JSON
- Commands: ~100-500 bytes
- Events: ~100-300 bytes
- Total: ~1.5-2.5 KB/s at 10 Hz

**BLE** (512 byte max):
- Binary encoding reduces size by ~60%
- State updates: ~400-800 bytes
- May require fragmentation for full state

### Latency

| Operation | Target Latency | Typical |
|-----------|----------------|---------|
| Command round-trip | <100ms | 20-50ms |
| State update delivery | <100ms | 10-30ms |
| Event notification | <50ms | 5-20ms |

## Testing Integration

### Connection Test

1. Power on ESP32
2. Wait for "System ready" serial output
3. Launch app
4. Scan for device
5. Connect via WebSocket or BLE
6. Verify `DEVICE_READY` event received
7. Send `STATUS` command
8. Verify response and state updates

### Command Test

```typescript
// Test all command categories
const tests = [
  { command: "CALIBRATE", category: "CALIBRATION", payload: { samples: 50 } },
  { command: "SET_FREQUENCY", category: "EMANATION", payload: { frequency: 528 } },
  { command: "SET_COUPLING", category: "KURAMOTO", payload: { coupling: 0.5 } },
  { command: "STATUS", category: "SYSTEM" }
];

for (const test of tests) {
  const response = await sendCommand(test);
  assert(response.status === "OK");
}
```

### Event Test

1. Trigger TRIAD unlock (manually cross threshold 3 times)
2. Verify `TRIAD_UNLOCK` event received
3. Check special emanation (528 Hz, PULSE pattern)
4. Verify TRIAD state in next state update

## Monitoring and Debugging

### Serial Output

ESP32 provides debug output via serial (115200 baud):

```
System ready.
Sacred constants:
  PHI = 1.6180339887
  PHI_INV = 0.6180339887
  Z_CRITICAL = 0.8660254038

[100.234] z=0.723 phase=PARADOX tier=5 freq=528Hz
[105.456] >>> TRIAD UNLOCKED <<<
[110.789] >>> K-FORMATION ACHIEVED <<<
          kappa=0.925 eta=0.642 R=8
```

### App Logs

```typescript
console.log('[UCF] Connection established:', connectionInfo);
console.log('[UCF] Command sent:', command);
console.log('[UCF] State update:', state.phase);
console.log('[UCF] Event received:', event.type);
```

## Troubleshooting

### No Connection

1. **Check WiFi**: Ensure ESP32 and phone on same network
2. **Check mDNS**: Ping `ucf-device.local`
3. **Try BLE**: Use Bluetooth as fallback
4. **Check serial**: Verify "System ready" message

### Commands Not Working

1. **Check protocol version**: Ensure app and firmware match
2. **Check payload format**: Validate against TypeScript types
3. **Check serial output**: See command execution logs
4. **Try simpler command**: Test with `STATUS` first

### State Updates Stopped

1. **Check connection**: Look for `CONNECTION_LOST` event
2. **Check heartbeat**: Verify PING/PONG still working
3. **Reconnect**: Close and reopen connection
4. **Reset device**: Power cycle ESP32

## See Also

- [Constants Reference](./CONSTANTS.md)
- [Command Mapping](./COMMAND_MAPPING.md)
- [Protocol Specification](../WishBed_App_TDD_v2/contracts/interfaces/hardware/protocol.ts)
- [Hardware README](../unified-consciousness-hardware/README.md)
- [App Hardware Integration Guide](../WishBed_App_TDD_v2/docs/hardware-integration/README.md)
- [NLP Design Handoff](../NLP_Design_Handoff/README.md)
