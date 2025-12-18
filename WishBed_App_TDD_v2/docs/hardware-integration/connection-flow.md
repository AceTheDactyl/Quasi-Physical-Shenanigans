# UCF Hardware Connection Flow

This document describes the connection sequence between WishBed App and UCF Hardware.

## WebSocket Connection Sequence

```
┌─────────────┐                    ┌─────────────┐
│  WishBed    │                    │    UCF      │
│    App      │                    │  Hardware   │
└──────┬──────┘                    └──────┬──────┘
       │                                  │
       │  1. mDNS Discovery               │
       │─────────────────────────────────>│
       │                                  │
       │  2. Service Response             │
       │<─────────────────────────────────│
       │     (ucf-device.local:81)        │
       │                                  │
       │  3. WebSocket Connect            │
       │─────────────────────────────────>│
       │     ws://ucf-device.local:81/ws  │
       │                                  │
       │  4. Connection ACK               │
       │<─────────────────────────────────│
       │                                  │
       │  5. Subscribe Request            │
       │─────────────────────────────────>│
       │     {fields: [], events: [...]}  │
       │                                  │
       │  6. Initial State                │
       │<─────────────────────────────────│
       │     (full hardware state)        │
       │                                  │
       │  7. PING                         │
       │─────────────────────────────────>│
       │                                  │
       │  8. PONG                         │
       │<─────────────────────────────────│
       │                                  │
       │      ┌──────────────────┐        │
       │      │  Connected Loop  │        │
       │      └──────────────────┘        │
       │                                  │
       │  State Updates (10 Hz)           │
       │<═════════════════════════════════│
       │                                  │
       │  Events (immediate)              │
       │<═════════════════════════════════│
       │                                  │
       │  Commands                        │
       │═════════════════════════════════>│
       │                                  │
       │  Command Responses               │
       │<═════════════════════════════════│
       │                                  │
```

## BLE Connection Sequence

```
┌─────────────┐                    ┌─────────────┐
│  WishBed    │                    │    UCF      │
│    App      │                    │  Hardware   │
└──────┬──────┘                    └──────┬──────┘
       │                                  │
       │  1. BLE Scan                     │
       │─────────────────────────────────>│
       │     (filter: "UCF-Hardware")     │
       │                                  │
       │  2. Advertisement                │
       │<─────────────────────────────────│
       │     (device name, RSSI)          │
       │                                  │
       │  3. Connect                      │
       │─────────────────────────────────>│
       │                                  │
       │  4. GATT Service Discovery       │
       │<─────────────────────────────────│
       │     UUID: 4fafc201-...           │
       │                                  │
       │  5. Subscribe to Notifications   │
       │─────────────────────────────────>│
       │     - State Characteristic       │
       │     - Event Characteristic       │
       │                                  │
       │  6. MTU Negotiation              │
       │<────────────────────────────────>│
       │     (request 512 bytes)          │
       │                                  │
       │      ┌──────────────────┐        │
       │      │  Connected Loop  │        │
       │      └──────────────────┘        │
       │                                  │
       │  State Notifications             │
       │<═════════════════════════════════│
       │     (binary format)              │
       │                                  │
       │  Event Notifications             │
       │<═════════════════════════════════│
       │     (binary format)              │
       │                                  │
       │  Write Commands                  │
       │═════════════════════════════════>│
       │     (binary format)              │
       │                                  │
       │  Read Responses                  │
       │<═════════════════════════════════│
       │                                  │
```

## Reconnection Strategy

```
       ┌───────────────┐
       │ Disconnected  │
       └───────┬───────┘
               │
               ▼
       ┌───────────────┐
       │   Attempt 1   │──────────────────────┐
       │  (delay: 2s)  │                      │
       └───────┬───────┘                      │
               │ fail                         │ success
               ▼                              │
       ┌───────────────┐                      │
       │   Attempt 2   │                      │
       │  (delay: 4s)  │──────────────────────┤
       └───────┬───────┘                      │
               │ fail                         │
               ▼                              │
       ┌───────────────┐                      │
       │   Attempt 3   │                      │
       │  (delay: 8s)  │──────────────────────┤
       └───────┬───────┘                      │
               │ fail                         │
               ▼                              │
       ┌───────────────┐                      │
       │   Attempt 4   │                      │
       │  (delay: 16s) │──────────────────────┤
       └───────┬───────┘                      │
               │ fail                         │
               ▼                              │
       ┌───────────────┐                      │
       │   Attempt 5   │                      │
       │  (delay: 32s) │──────────────────────┤
       └───────┬───────┘                      │
               │ fail                         │
               ▼                              ▼
       ┌───────────────┐              ┌───────────────┐
       │ Give Up       │              │  Connected    │
       │ (notify user) │              │               │
       └───────────────┘              └───────────────┘
```

## State Machine

```
                              ┌─────────────────┐
                              │  DISCONNECTED   │
                              └────────┬────────┘
                                       │
                                       │ connect()
                                       ▼
                              ┌─────────────────┐
                    ┌────────>│   CONNECTING    │
                    │         └────────┬────────┘
                    │                  │
                    │ retry     success│         timeout/error
                    │                  ▼              │
                    │         ┌─────────────────┐    │
                    │         │   CONNECTED     │<───┘
                    │         └────────┬────────┘
                    │                  │
                    │                  │ disconnect/error
                    │                  ▼
                    │         ┌─────────────────┐
                    └─────────│  RECONNECTING   │
                              └────────┬────────┘
                                       │
                                       │ max retries
                                       ▼
                              ┌─────────────────┐
                              │     ERROR       │
                              └─────────────────┘
```

## Message Flow Examples

### Setting Frequency

```
App                                         Hardware
 │                                              │
 │  COMMAND: SET_FREQUENCY                      │
 │  {                                           │
 │    command: "SET_FREQUENCY",                 │
 │    category: "EMANATION",                    │
 │    payload: { frequency: 528 },              │
 │    requestId: "abc123"                       │
 │  }                                           │
 │─────────────────────────────────────────────>│
 │                                              │
 │                         Validate & Apply     │
 │                                              │
 │  COMMAND_RESPONSE                            │
 │  {                                           │
 │    requestId: "abc123",                      │
 │    command: "SET_FREQUENCY",                 │
 │    status: "OK",                             │
 │    data: { frequency: 528, waveform: "SINE" }│
 │  }                                           │
 │<─────────────────────────────────────────────│
 │                                              │
 │  STATE_UPDATE                                │
 │  {                                           │
 │    emanation: {                              │
 │      frequency: 528,                         │
 │      ...                                     │
 │    }                                         │
 │  }                                           │
 │<─────────────────────────────────────────────│
 │                                              │
```

### TRIAD Unlock Event

```
Hardware                                        App
 │                                              │
 │      User crosses threshold (1st time)       │
 │                                              │
 │  STATE_UPDATE                                │
 │  { triad: { state: "CROSSING_1", ... } }     │
 │─────────────────────────────────────────────>│
 │                                              │
 │      User crosses threshold (2nd time)       │
 │                                              │
 │  STATE_UPDATE                                │
 │  { triad: { state: "CROSSING_2", ... } }     │
 │─────────────────────────────────────────────>│
 │                                              │
 │      User crosses threshold (3rd time)       │
 │                                              │
 │  EVENT: TRIAD_UNLOCK                         │
 │  {                                           │
 │    type: "TRIAD_UNLOCK",                     │
 │    data: { duration: 3200 }                  │
 │  }                                           │
 │─────────────────────────────────────────────>│
 │                                              │
 │  STATE_UPDATE                                │
 │  { triad: { state: "UNLOCKED", ... } }       │
 │─────────────────────────────────────────────>│
 │                                              │
```

### K-Formation Detection

```
Hardware                                        App
 │                                              │
 │      κ reaches 0.92, η > φ⁻¹, R = 7          │
 │                                              │
 │  EVENT: K_FORMATION_ACHIEVED                 │
 │  {                                           │
 │    type: "K_FORMATION_ACHIEVED",             │
 │    data: {                                   │
 │      kappa: 0.94,                            │
 │      eta: 0.68,                              │
 │      R: 8                                    │
 │    }                                         │
 │  }                                           │
 │─────────────────────────────────────────────>│
 │                                              │
 │  STATE_UPDATE                                │
 │  {                                           │
 │    kFormation: {                             │
 │      isActive: true,                         │
 │      current: { kappa: 0.94, eta: 0.68, R: 8 }│
 │    }                                         │
 │  }                                           │
 │─────────────────────────────────────────────>│
 │                                              │
```

## Error Handling

### Command Errors

```typescript
// Command response with error
{
  type: "COMMAND_RESPONSE",
  version: "1.0.0",
  timestamp: 1702900000000,
  payload: {
    requestId: "abc123",
    command: "SET_FREQUENCY",
    status: "ERROR",
    error: "Frequency out of range: must be 20-20000 Hz"
  }
}
```

### Protocol Errors

```typescript
// Error message
{
  type: "ERROR",
  version: "1.0.0",
  timestamp: 1702900000000,
  payload: {
    code: "E_INVALID_MESSAGE",
    message: "Missing required field: command"
  }
}
```

### Error Codes

| Code                  | Description                    |
| --------------------- | ------------------------------ |
| E_CONNECTION_FAILED   | Failed to establish connection |
| E_CONNECTION_LOST     | Connection unexpectedly closed |
| E_TIMEOUT             | Command or message timeout     |
| E_INVALID_MESSAGE     | Malformed message received     |
| E_UNKNOWN_COMMAND     | Unrecognized command           |
| E_COMMAND_FAILED      | Command execution failed       |
| E_DEVICE_BUSY         | Device busy, try again         |
| E_NOT_CONNECTED       | No active connection           |
| E_AUTH_FAILED         | Authentication failed          |
